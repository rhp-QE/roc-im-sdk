#include "imsdk/src/core/message/MessageManager.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/message/opt/db_opt/db_opt.h"

namespace roc::imsdk::core {

MessageManager::MessageManager() = default;

MessageManager::~MessageManager() = default;

/// 保存网络消息
std::vector<std::shared_ptr<model::MessageModel>> MessageManager::save_net_msgs(std::vector<const network::MsgData *> msgs) {

    // 转换为 db 消息
    auto db_msgs = base::util::transform(msgs, [this](const network::MsgData *msg) {
        return convert_net_msg_to_db_msg(msg);
    });

    // 保存到数据库
    bool ret = message::dbopt::insert_message(w_sdk_root_, db_msgs);
    if (!ret) {
        return {};
    }

    // 转换为 sdk 消息
    auto sdk_msgs = base::util::transform(db_msgs, [this](const std::shared_ptr<core::message::MessageORM> &msg) {
        return convert_db_msg_to_sdk_msg(msg.get());
    });

    // 保存到缓存
    for (const auto &sdk_msg : sdk_msgs) {
        msg_cache_[sdk_msg->client_msg_id()] = sdk_msg;
    }

    return sdk_msgs;
}

/// 设置 sdk 消息
void MessageManager::set_sdk_msg(const core::message::MessageORM *db_msg) {
    if (!db_msg) {
        return;
    }

    msg_cache_[db_msg->client_msg_id] = convert_db_msg_to_sdk_msg(db_msg);
}

/// 消息转换 网络消息 -> db 消息
std::shared_ptr<core::message::MessageORM> MessageManager::convert_net_msg_to_db_msg(const network::MsgData *msg) {
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
std::shared_ptr<model::MessageModel> MessageManager::convert_db_msg_to_sdk_msg(const core::message::MessageORM *db_msg) {
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


} // namespace roc::imsdk::core