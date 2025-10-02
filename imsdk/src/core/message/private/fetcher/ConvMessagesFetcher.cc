#include "ConvMessagesFetcher.h"

#include "imsdk/src/core/common/logger_macro.h"
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
std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(CONTEXT_T, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchConvMessageListReq>();

    req->set_convid(conv_id);
    req->set_cursor(range.first);
    req->set_limit(range.second - range.first + 1);
    req->set_forward(false);

    return req;
}

// 处理返回数据
void p_handle_fetch_conv_messgae_list_resp(CONTEXT_T, std::unique_ptr<network::FetchConvMessageListResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    std::vector<std::shared_ptr<network::MsgData>> net_msgs;

    while (!resp->messages().empty()) {
        auto msg = resp->mutable_messages()->ReleaseLast();
        net_msgs.push_back(std::shared_ptr<network::MsgData>(msg));
    }

    LOG_INFO("MsgManager", "call_track_id: {}, handle_fetch_conv_message_list_resp, size: {}", TRACK_ID, net_msgs.size());

    // 保存消息
    boost::asio::co_spawn(sdk_root->net_io_context(), message::ReceiveMessage::handle_receive_message(CONTEXT_V, net_msgs), boost::asio::detached);
}

asio::awaitable<void> ConvMessagesFetcher::fetch_conv_message_list_for_range(CONTEXT_T, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);
    
    // 防止死循环
    int cnt = 10;
    bool has_more = true;
    do {
        std::unique_ptr<network::FetchConvMessageListReq> req = p_make_fetch_conv_message_list_req(CONTEXT_V, conv_id, range);
        if (!req) {
            break;
        }

        // 发送请求
        std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error> resp = 
            co_await network::request::fetch_conv_message_list(CONTEXT_V, req.get());
        if (!resp.value()) {
            continue;
        }
        has_more = resp.value()->havemore();
        
        // 处理数据
        p_handle_fetch_conv_messgae_list_resp(CONTEXT_V, std::move(resp.value()));

    } while(cnt-- > 0 && has_more);
}

// ==========================================================================================================

boost::asio::awaitable<void> ConvMessagesFetcher::fetch_conv_message_list(CONTEXT_T, std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_empty_ranges = message::SaveMessage::empty_message_range_for_conv_id(CONTEXT_V, conv_id);

    for (const auto &range : msg_empty_ranges) {

        LOG_INFO("MsgManager", "start_fetch_conv_message_list, conv_id: {}, range: {{{}, {}}}", conv_id, range.first, range.second);

        co_await fetch_conv_message_list_for_range(CONTEXT_V, conv_id, range);
    }

    std::cout<<"fetch conv message success"<<std::endl;
}

// ==========================================================================================================

} // namespace roc::imsdk::core::message