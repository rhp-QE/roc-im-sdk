///
/// @file   MessageSendLogic.cc
/// @brief  消息发送逻辑
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/message/MessageSendLogic.h"
#include "base/network/include/Error.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include <cstdint>
#include <expected>
#include <memory>
#include <string>

namespace roc::imsdk::service {

// ----------------------------- no member private method -----------------------------
// 发送消息到服务器
boost::asio::awaitable<std::expected<void, roc::error::Error>>
p_send_message_to_server(std::shared_ptr<SDKRoot> root, std::vector<std::shared_ptr<model::MessageModel>> messages);

// 将 message 转为 sdkws.message
void p_message_to_sdkws_message(std::shared_ptr<model::MessageModel> message, network::MsgData *sdkws_message);

// -------------------------------------------------------------------------------------

MessageSendLogic::MessageSendLogic(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {
}

MessageSendLogic::~MessageSendLogic() {
}

// 发送消息
boost::asio::awaitable<std::expected<void, roc::error::Error>>
MessageSendLogic::send_message(std::vector<std::shared_ptr<model::MessageModel>> messages) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root_, std::unexpected(roc::error::make_error(4000000, "[MessageServiceCore:send_message]" "SDKRoot is nullptr")));

    // message 合法性检验
    // 将 message 转为 databaseModel 并保存

    // 将 message 转为 sdkws.message 并发送
    co_await p_send_message_to_server(sdk_root, messages);
}


// ----------------------------- no member private method -----------------------------

boost::asio::awaitable<std::expected<void, roc::error::Error>> p_send_message_to_server(std::shared_ptr<SDKRoot> root, std::vector<std::shared_ptr<model::MessageModel>> messages) {
    // 将 消息转为 sdkws.message 并发送
    std::unique_ptr<network::SendMessageReq> req = std::make_unique<network::SendMessageReq>();
    for (auto &message : messages) {
        p_message_to_sdkws_message(message, req->add_msgs());
    }

    // 发送消息
    std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error> resp = co_await network::request::send_message(root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return std::unexpected(resp.error());
    }

    // 广播消息发送成功事件
    co_return std::expected<void, roc::error::Error>();
}

void p_message_to_sdkws_message(std::shared_ptr<model::MessageModel> message, network::MsgData *sdkws_message) {
    if (!message || !sdkws_message) {
        return;
    }

    sdkws_message->set_sendid(message->from_user_id());
    sdkws_message->set_recvid(message->to_user_id());
    sdkws_message->set_convid(message->conversation_id());
    sdkws_message->set_content(message->content());
}

// -------------------------------------------------------------------------------------

} // namespace roc::imsdk