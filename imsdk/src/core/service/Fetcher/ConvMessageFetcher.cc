///
/// @file   ConvMessageFetcher.cc
/// @brief  会话消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/ConvMessageFetcher.h"
#include "base/utils/utils.h"
#include "imsdk/src/core/injection/Injection.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/utils/Utils.h"
#include "imsdk/src/core/service/message/SaveMessage.h"
#include "imsdk/src/core/service/Range/MessageRange.h"
#include <memory>

namespace roc::imsdk::service {

// =========================================================================================================

// 生成请求
std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(SDKRoot *root, std::string conv_id, std::pair<int64_t, int64_t> range) {
    auto req = std::make_unique<network::FetchConvMessageListReq>();

    req->set_convid(conv_id);
    req->set_cursor(range.first);
    req->set_limit(range.second - range.first + 1);
    req->set_forward(false);

    return req;
}

// 处理返回数据
void p_handle_fetch_conv_messgae_list_resp(std::weak_ptr<SDKRoot> w_sdk_root, std::unique_ptr<network::FetchConvMessageListResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    std::vector<const network::MsgData *> msgs;
    for (auto &msg : resp->messages()) {
        msgs.push_back(&msg);
    }

    // 保存并获取sdk_msg
    auto sdk_msgs = SaveMessage::save_net_message(w_sdk_root, msgs);

    // 转换为conv_msg_union
    auto conv_msg_union = util::convert_sdk_msg_to_conv_msgs_union(w_sdk_root, sdk_msgs);

    // 向上抛 conv_msg_union
    base::util::safe_invoke_block(sdk_root->injection()->on_new_message_callback, conv_msg_union);
}

/// 获取空缺区间
std::vector<std::pair<int64_t, int64_t>> p_get_msg_empty_ranges(std::vector<std::pair<int64_t, int64_t>> &ranges) {
    if (ranges.empty()) {
        return {};
    }

    std::vector<std::pair<int64_t, int64_t>> empty_ranges;
 
    std::pair<int64_t, int64_t> last_range = {0, 0};

    for (const auto &range : ranges) {
        std::pair<int64_t, int64_t> new_range = {last_range.second + 1, range.first - 1};
        if (new_range.first <= new_range.second) {
            empty_ranges.push_back(new_range);
        }
        last_range = range;
    }

   return empty_ranges;
}

// ==========================================================================================================


// ================================== implement public method (begin) =======================================

ConvMessageFetcher::ConvMessageFetcher(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {}

ConvMessageFetcher::~ConvMessageFetcher() = default;

asio::awaitable<void> ConvMessageFetcher::fetch_conv_message_list(std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root_);

    auto msg_ranges = sdk_root->message_range()->message_range_for_conv_id(conv_id);
    auto msg_empty_ranges = p_get_msg_empty_ranges(msg_ranges);

    for (const auto &range : msg_empty_ranges) {
        co_await fetch_conv_message_list_for_range(conv_id, range);
    }
}

asio::awaitable<void> ConvMessageFetcher::fetch_conv_message_list_for_range(std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root_);
    
    // 防止死循环
    int cnt = 10;
    do {
        std::unique_ptr<network::FetchConvMessageListReq> req = p_make_fetch_conv_message_list_req(sdk_root.get(), conv_id, range);
        if (!req) {
            break;
        }

         // 发送请求
        std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error> resp = 
            co_await network::request::fetch_conv_message_list(sdk_root.get(), req.get());
        if (!resp.value()) {
            continue;
        }
        
        // 处理数据
        p_handle_fetch_conv_messgae_list_resp(w_sdk_root_, std::move(resp.value()));

    } while(cnt-- > 0);
}

// ================================== implement public method (end) =========================================

} // namespace roc::imsdk::service