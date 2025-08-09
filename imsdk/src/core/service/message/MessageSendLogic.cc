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
#include "imsdk/src/core/utils/Utils.h"
#include <cstdint>
#include <expected>
#include <memory>
#include <string>

namespace roc::imsdk::service {

// ----------------------------- no member private method -----------------------------

void p_message_to_sdkws_message(std::shared_ptr<model::MessageModel> message, network::MsgData *sdkws_message) {
    if (!message || !sdkws_message) {
        return;
    }

    sdkws_message->set_sendid(message->from_user_id());
    sdkws_message->set_recvid(message->to_user_id());
    sdkws_message->set_convid(message->conversation_id());
    sdkws_message->set_content(message->content());
}

boost::asio::awaitable<std::expected<void, roc::error::Error>> p_send_message_to_server(std::weak_ptr<SDKRoot> w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>> messages) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(4000000, "[MessageServiceCore:send_message_to_conv]" "SDKRoot is nullptr")));

    // 将 消息转为 sdkws.message
    std::unique_ptr<network::SendMessageReq> req = std::make_unique<network::SendMessageReq>();
    for (auto &message : messages) {
        p_message_to_sdkws_message(message, req->add_msgs());
    }

    // 发送消息
    std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error> resp = co_await network::request::send_message(sdk_root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return std::unexpected(resp.error());
    }

    // 广播消息发送成功事件
    co_return std::expected<void, roc::error::Error>();
}

// -------------------------------------------------------------------------------------

boost::asio::awaitable<message::SendMessageResult> message::send_message_v2(std::weak_ptr<SDKRoot> w_sdk_root, std::vector<std::shared_ptr<service::SendMessageModel>> send_models) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, message::SendMessageResult());

    // 消息合法性校验

    // 将 send_model 转为 db_message 并保存
    std::vector<db::MessageORM *> db_msg_vec = base::util::transform(send_models, [w_sdk_root](const std::shared_ptr<service::SendMessageModel> &send_model) -> std::unique_ptr<db::MessageORM> {
        return util::convert_send_model_to_db_msg(w_sdk_root, send_model);
    });

    // 保存 db_message 并 获取对应 sdk_msg
    auto sdk_msg_vec = sdk_root->message_cache()->update_and_get_sdk_message(db_msg_vec);

    // 将 sdk_msg 转为 sdkws.message
    std::unique_ptr<network::SendMessageReq> req = std::make_unique<network::SendMessageReq>();
    for (auto &sdk_msg : sdk_msg_vec) {
        util::convert_sdk_msg_to_sdkws_msg(sdk_msg, req->add_msgs());
    }

    // 发送消息
    std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error> resp = co_await network::request::send_message(sdk_root.get(), req.get());

    co_return message::SendMessageResult{sdk_msg_vec, {}, nullptr};
}


//============================ implment public method (begin) ============================
MessageSendLogic::MessageSendLogic(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {
}

MessageSendLogic::~MessageSendLogic() {
}

// 发送消息
boost::asio::awaitable<std::expected<void, roc::error::Error>>
MessageSendLogic::send_message(std::vector<std::shared_ptr<model::MessageModel>> messages) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root_, std::unexpected(roc::error::make_error(4000000, "[MessageServiceCore:send_message]" "SDKRoot is nullptr")));

    // message 合法性检验
    // 将 message 转为 db_message 并保存
    
    // 将 message 转为 sdkws.message 并发送
    co_await p_send_message_to_server(sdk_root, messages);
}

// ============================ implment public method (end) ============================


} // namespace roc::imsdk