#include "ConvMessagesFetcher.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/convert.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"

#include "base/utils/utils.h"

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

    std::vector<std::shared_ptr<network::MsgData>> net_msgs;

    while (!resp->messages().empty()) {
        auto msg = resp->mutable_messages()->ReleaseLast();
        net_msgs.push_back(std::shared_ptr<network::MsgData>(msg));
    }

    // 保存消息
    message::ReceiveMessage::handle_receive_message(w_sdk_root, net_msgs);
}

asio::awaitable<void> ConvMessagesFetcher::fetch_conv_message_list_for_range(W_SDK_ROOT, std::string conv_id, std::pair<int64_t, int64_t> range) {
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

boost::asio::awaitable<void> ConvMessagesFetcher::fetch_conv_message_list(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_empty_ranges = message::SaveMessage::empty_message_range_for_conv_id(w_sdk_root, conv_id);

    for (const auto &range : msg_empty_ranges) {
        co_await fetch_conv_message_list_for_range(w_sdk_root, conv_id, range);
    }
}

// ==========================================================================================================

} // namespace roc::imsdk::core::message