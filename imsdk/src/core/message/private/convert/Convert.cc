#include "Convert.h"


namespace roc::imsdk::core::message {

/// 消息转换 网络消息 -> db 消息
std::shared_ptr<core::message::MessageORM> Convert::convert_net_msg_to_db_msg(const network::MsgData *msg) {
    if (!msg) {
        return nullptr;
    }
    std::shared_ptr<core::message::MessageORM> db_msg = std::make_shared<core::message::MessageORM>();
    db_msg->content = msg->content();
    db_msg->client_msg_id = msg->clientmsgid();
    db_msg->server_msg_id = msg->servermsgid();
    db_msg->conv_id = msg->convid();
    db_msg->sender_id = msg->sendid();
    db_msg->send_time = msg->sendtime();
    db_msg->is_deleted = msg->isdeleted(); 
    db_msg->is_recalled = msg->isrecalled();

    //ext
    db_msg->ext = msg->ex();
    db_msg->server_index = msg->serverordindex();
    db_msg->client_index = msg->seq();

    return db_msg;
}

/// 消息转换 db 消息 -> sdk 消息
std::shared_ptr<model::MessageModel> Convert::convert_db_msg_to_sdk_msg(const core::message::MessageORM *db_msg) {
    if (!db_msg) {
        return nullptr;
    }
    std::shared_ptr<model::MessageModel> sdk_msg = std::make_shared<model::MessageModel>();
    sdk_msg->content_ = db_msg->content;
    sdk_msg->client_msg_id_ = db_msg->client_msg_id;
    sdk_msg->server_msg_id_ = db_msg->server_msg_id;
    sdk_msg->conversation_id_ = db_msg->conv_id;
    sdk_msg->from_user_id_ = db_msg->sender_id;
    sdk_msg->client_order_index_ = db_msg->client_index;
    sdk_msg->server_order_index_ = db_msg->server_index;
    return sdk_msg;
}

} // namespace roc::imsdk::core::message
