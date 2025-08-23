#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include <memory>

namespace roc::imsdk::core::conversation {

struct FetchUserMessageResult {

};

asio::awaitable<void> UserMessageFetcher::fetch_user_messages(W_SDK_ROOT) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    // auto conv_manager = sdk_root->conversation_manager();

    // // 获取最新游标
    // int64_t cursor = conv_manager->cursor();

    // 构造请求
    std::unique_ptr<network::FetchUserMessageListReq> req = make_fetch_user_message_list_req(w_sdk_root, -1);

    // 发送请求
    std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error> resp = co_await network::request::fetch_user_message_list(sdk_root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return;
    }

    std::cout<<"fetch user message success"<<std::endl;

    co_await handle_fetched_user_message(w_sdk_root, std::move(resp.value()));

    co_return;
}

// private static methods ------------------------------------------------------------

std::unique_ptr<network::FetchUserMessageListReq> UserMessageFetcher::make_fetch_user_message_list_req(W_SDK_ROOT, int64_t cursor) {
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

boost::asio::awaitable<void> UserMessageFetcher::handle_fetched_user_message(W_SDK_ROOT, std::unique_ptr<network::FetchUserMessageListResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_manager = sdk_root->message_manager();
    auto conv_manager = sdk_root->conversation_manager();

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

    // 保存消息和会话
    msg_manager->handle_receive_message(net_msgs);

    conv_manager->save_net_convs(net_convs);

    // 更新游标
    conv_manager->set_cursor(resp->stop());

    // 上抛
    co_return;
}

} // namespace roc::imsdk::core::conversation