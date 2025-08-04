#pragma once

#include "imsdk/src/core/db/model/ConversationORM.h"
#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include <memory>
#include <string>

namespace roc::imsdk {

inline std::string key_for_user(const std::string& user_id, const std::string& key) {
    return user_id + "_" + key;
}

inline std::unique_ptr<db::ConversationORM> convert_net_conv_to_db_conv(const network::ConversationInfo *conv) {
    auto db_conv = std::make_unique<db::ConversationORM>();
    
    db_conv->conv_id = conv->convid();
    db_conv->conv_type = conv->convtype();
    db_conv->name = conv->convname();
    db_conv->avatar = conv->convavatar();
    db_conv->unread_count = conv->convunreadcount();
    db_conv->last_message_server_id = conv->lastmsg().servermsgid();
    db_conv->last_message_clent_id = conv->lastmsg().clientmsgid();
    db_conv->is_top = conv->istop();
    db_conv->is_muted = conv->ismuted();
    db_conv->is_deleted = conv->isdelete();

    return db_conv;
}

inline std::unique_ptr<db::MessageORM> convert_net_msg_to_db_msg(const network::MsgData *msg) {
    auto db_msg = std::make_unique<db::MessageORM>();
    db_msg->server_msg_id = msg->servermsgid();
    db_msg->client_msg_id = msg->clientmsgid();
    db_msg->server_index = msg->seq();
    db_msg->is_deleted = msg->isdeleted();
    db_msg->is_recalled = msg->isrecalled();
    db_msg->send_time = msg->sendtime();
    db_msg->content = msg->content();
    db_msg->ext = msg->ex();
    db_msg->conv_id = msg->convid();
    db_msg->content = msg->content();

    return db_msg;
}

}