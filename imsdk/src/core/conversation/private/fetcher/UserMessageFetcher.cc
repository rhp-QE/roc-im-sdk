#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/save/SaveConversation.h"
#include "imsdk/src/core/conversation/private/receive/ReceiveConversation.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/base/include/network/Error.h"

#include <memory>
#include <atomic>
#include <expected>

namespace roc::imsdk::core::conversation {

struct FetchUserMessageResult {};

UserMessageFetcher::UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root)
    : w_sdk_root(sdk_root) {}

asio::awaitable<void> UserMessageFetcher::FetchUserMessages(CONTEXT_T) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    // auto conv_manager = sdk_root->conversation_manager();

    // // 获取最新游标
    // int64_t cursor = conv_manager->cursor();

    // 构造请求
    std::unique_ptr<network::FetchUserMessageListReq> req = p_MakeFetchUserMessageListReq(CONTEXT_V, -1);

    // 发送请求
    std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error> resp = co_await p_Request(CONTEXT_V, req.get());
    if (!resp || !resp.has_value()) {
        co_return;
    }

    // std::cout<<"fetch user message success"<<std::endl;

    co_await p_HandleFetchedUserMessage(CONTEXT_V, std::move(resp.value()));

    co_return;
}

// =================================== private ===========================================================

boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
UserMessageFetcher::p_Request(CONTEXT_T, network::FetchUserMessageListReq *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(network::SDKRequestType::FETCH_USER_MESSAGE_LIST));
    req->set_data(request->SerializeAsString());
    req->set_trackid(TRACK_ID);

    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await sdk_root->ConnectionManager()->SendRequest(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list response is empty"));
    }

    auto resp = std::make_unique<network::FetchUserMessageListResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list parse response failed"));
    }

    co_return resp;
}

std::unique_ptr<network::FetchUserMessageListReq> UserMessageFetcher::p_MakeFetchUserMessageListReq(CONTEXT_T, int64_t cursor) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchUserMessageListReq>();

    req->set_userid(sdk_root->config().user_id);
    req->set_limit(40);
    req->set_forward(true);
    
    if (cursor != -1) {
        req->set_news(false);
        req->set_cursor(cursor);
    } else {
        req->set_news(true);
    }

    return req;
}

boost::asio::awaitable<void> UserMessageFetcher::p_HandleFetchedUserMessage(CONTEXT_T, std::unique_ptr<network::FetchUserMessageListResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    std::vector<std::shared_ptr<network::ConversationInfo>> net_convs;
    std::vector<std::shared_ptr<network::MsgData>> net_msgs;

    auto convs = resp->mutable_convsinfo();

    while (!convs->empty()) {
        auto conv = convs->ReleaseLast();

        auto msgs = conv->mutable_msgs();
        while (!msgs->empty()) {
            auto msg = msgs->ReleaseLast();
            if (msg->iscmd()) {
                continue;
            }
            net_msgs.push_back(std::shared_ptr<network::MsgData>(msg->release_msg()));
        }

        net_convs.push_back(std::shared_ptr<network::ConversationInfo>(conv));
    }

    LOG_INFO("ConvManager", "finish_fetch_user_message, net_msgs: {}, net_convs: {}", net_msgs.size(), net_convs.size());

    // 处理接收到的消息
    sdk_root->MessageManager()->HandleReceiveMessage(CONTEXT_V, net_msgs);

    // 处理接收到的会话
    auto conv_manager = sdk_root->ConversationManager();
    co_spawn(sdk_root->sdk_io_context(), conv_manager->receive_conversation->HandleReceiveConversation(CONTEXT_V, std::move(net_convs)), asio::detached);

    // // 更新游标
    // conv_manager->set_cursor(resp->stop());

    // 上抛
    co_return;
}

} // namespace roc::imsdk::core::conversation