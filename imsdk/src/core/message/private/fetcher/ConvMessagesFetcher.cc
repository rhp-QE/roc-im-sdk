#include "ConvMessagesFetcher.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/convert.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/request/SDKRequest.h"

namespace roc::imsdk::core::message {

// 生成请求
std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(W_SDK_ROOT, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchConvMessageListReq>();

    req->set_convid(conv_id);
    req->set_cursor(range.first);
    req->set_limit(range.second - range.first + 1);
    req->set_forward(false);

    return req;
}

// 处理返回数据
void p_handle_fetch_conv_messgae_list_resp(W_SDK_ROOT, std::unique_ptr<network::FetchConvMessageListResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    auto msg_manager = sdk_root->message_manager();

    std::vector<const network::MsgData *> msgs;

    for (auto &msg : resp->messages()) {
        msgs.push_back(&msg);
    }

    // 保存消息
    msg_manager->save_net_msgs(msgs);

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

asio::awaitable<void> fetch_conv_message_list_for_range(W_SDK_ROOT, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);
    
    // 防止死循环
    int cnt = 10;
    do {
        std::unique_ptr<network::FetchConvMessageListReq> req = p_make_fetch_conv_message_list_req(w_sdk_root, conv_id, range);
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
        p_handle_fetch_conv_messgae_list_resp(w_sdk_root, std::move(resp.value()));

    } while(cnt-- > 0);
}

// ==========================================================================================================

boost::asio::awaitable<void> fetch_conv_message_list(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_manager = sdk_root->message_manager();

    auto msg_ranges = msg_manager->message_range_for_conv_id(conv_id);
    auto msg_empty_ranges = p_get_msg_empty_ranges(msg_ranges);

    for (const auto &range : msg_empty_ranges) {
        co_await fetch_conv_message_list_for_range(w_sdk_root, conv_id, range);
    }
}

// ==========================================================================================================



}