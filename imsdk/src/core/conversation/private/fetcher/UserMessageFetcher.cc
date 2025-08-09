#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/conversation/ConversationManager.h"

namespace roc::imsdk::core::conversation {

struct FetchUserMessageResult {

};

// private function declare ----------------------------------------------------------
std::unique_ptr<network::FetchUserMessageListReq> p_make_fetch_user_message_list_req(W_SDK_ROOT, int64_t cursor);
boost::asio::awaitable<void> handle_fetched_user_message(W_SDK_ROOT, std::unique_ptr<network::FetchUserMessageListResp> resp);
// ----------------------------------------------------------------------------------


asio::awaitable<void> fetch_user_messages(W_SDK_ROOT, std::string user_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto conv_manager = sdk_root->conversation_manager();

    // 获取最新游标
    int64_t cursor = conv_manager->cursor();

    // 构造请求
    std::unique_ptr<network::FetchUserMessageListReq> req = p_make_fetch_user_message_list_req(w_sdk_root, cursor);

    // 发送请求
    std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error> resp = co_await network::request::fetch_user_message_list(sdk_root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return;
    }

    co_await handle_fetched_user_message(w_sdk_root, std::move(resp.value()));

    co_return;
}

// private function impl ------------------------------------------------------------

std::unique_ptr<network::FetchUserMessageListReq> p_make_fetch_user_message_list_req(W_SDK_ROOT, int64_t cursor) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchUserMessageListReq>();

    req->set_userid(sdk_root->config().user_id);
    req->set_cursor(cursor);
    req->set_limit(20);
    req->set_forward(true);
    req->set_news(true);

    return req;
}

boost::asio::awaitable<void> handle_fetched_user_message(W_SDK_ROOT, std::unique_ptr<network::FetchUserMessageListResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_manager = sdk_root->message_manager();
    auto conv_manager = sdk_root->conversation_manager();

    std::vector<const network::ConversationInfo *> net_convs;
    std::vector<const network::MsgData *> net_msgs;


    for (auto &conv : resp->convsinfo()) {
        net_convs.push_back(&conv);
        for (auto &msg : conv.msgs()) {
            if (msg.iscmd()) {
                continue;
            }
            net_msgs.push_back(&(msg.msg()));
        }
    }

    // 保存消息和会话
    msg_manager->save_net_msgs(net_msgs);
    conv_manager->save_net_convs(net_convs);

    // 更新游标
    conv_manager->set_cursor(resp->cursor());

    // 上抛
    co_return;
}

} // namespace roc::imsdk::core::conversation