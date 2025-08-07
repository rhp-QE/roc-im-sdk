#pragma once

#include "base/utils/utils.h"
#include "imsdk/src/core/db/model/ConversationORM.h"
#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/injection/Injection.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/service/message/MessageSendLogic.h"
#include <memory>
#include <string>

namespace roc::imsdk::util {

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

inline std::vector<injection::ConvMessagesUnion> convert_sdk_msg_to_conv_msg_union(std::weak_ptr<SDKRoot> w_sdk_root, const std::vector<std::shared_ptr<model::MessageModel>> &sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, std::vector<injection::ConvMessagesUnion>())

    std::unordered_map<std::string, std::vector<std::shared_ptr<model::MessageModel>>> message_map
        = base::util::group_by_key(sdk_msgs, [](const std::shared_ptr<model::MessageModel> &sdk_msg) -> std::string {
            return sdk_msg->conversation_id();
        });

    std::vector<injection::ConvMessagesUnion> conv_msg_union;

    for (auto &value : message_map) {
        conv_msg_union.emplace_back(injection::ConvMessagesUnion{
            sdk_root->conversation_cache()->get_sdk_conv(value.first),
            value.second
        });
    }

    return conv_msg_union;
}

inline std::unique_ptr<db::MessageORM> convert_send_model_to_db_msg(std::weak_ptr<SDKRoot> w_sdk_root, const std::shared_ptr<service::SendMessageModel> &send_model) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, std::unique_ptr<db::MessageORM>());

    auto db_msg = std::make_unique<db::MessageORM>();
    db_msg->conv_id = send_model->conv_id;
    db_msg->content = send_model->content;
    db_msg->ext = send_model->sync_etx;
    db_msg->sender_id = sdk_root->config().user_id;

    return db_msg;
}

inline void convert_sdk_msg_to_sdkws_msg(const std::shared_ptr<model::MessageModel> &sdk_msg, network::MsgData *sdkws_msg) {
    if (!sdkws_msg || !sdkws_msg) {
        return;
    }

    sdkws_msg->set_sendid(sdk_msg->from_user_id());
    sdkws_msg->set_recvid(sdk_msg->to_user_id());
    sdkws_msg->set_convid(sdk_msg->conversation_id());
    sdkws_msg->set_content(sdk_msg->content());
    
}

}