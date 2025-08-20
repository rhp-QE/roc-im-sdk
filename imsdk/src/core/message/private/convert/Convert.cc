#include "Convert.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/MessageManager.h"



namespace roc::imsdk::core::message {

/// 消息转换 网络消息 -> db 消息
std::shared_ptr<core::message::MessageORM> Convert::convert_net_msg_to_db_msg(const network::MsgData *msg) {
    if (!msg) {
        return nullptr;
    }
    std::shared_ptr<core::message::MessageORM> db_msg = std::make_shared<core::message::MessageORM>();
    
    // Status and flags
    db_msg->status = msg->status();
    
    db_msg->is_pinned = msg->ispinned();
    
    db_msg->is_deleted = msg->isdeleted(); 
    
    db_msg->is_recalled = msg->isrecalled();
    
    db_msg->is_group_msg = msg->isgroupmsg();
    
    // Message content
    db_msg->content = msg->content();
    
    // User IDs
    db_msg->to_user_id = msg->recvid();
    
    db_msg->from_user_id = msg->sendid();
    
    // Message IDs
    db_msg->client_msg_id = msg->clientmsgid();
    
    db_msg->server_msg_id = msg->servermsgid();
    
    // Conversation ID
    db_msg->conversation_id = msg->convid();
    
    // Order indices
    db_msg->client_order_index = 0; // TODO: Set from network message if available
    
    db_msg->server_order_index = msg->seq();
    
    // Timestamps
    db_msg->client_send_time = 0; // TODO: Set from network message if available
    
    db_msg->server_send_time = msg->sendtime();
    
    // Extensions
    db_msg->sync_ext = msg->syncext();
    
    db_msg->local_ext = ""; // TODO: Set from network message if available
    
    return db_msg;
}

/// 消息转换 db 消息 -> sdk 消息
/// sdk 消息需要确保全局实例唯一性， 要从 cache 内查， 没有再构造
std::shared_ptr<model::MessageModel> Convert::convert_db_msg_to_sdk_msg(W_SDK_ROOT, const core::message::MessageORM *db_msg) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    if (!db_msg || db_msg->client_msg_id.empty()) {
        return nullptr;
    }

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, nullptr);

    std::shared_ptr<model::MessageModel> sdk_msg;
    if (msg_manager->msg_cache_.find(db_msg->client_msg_id) != msg_manager->msg_cache_.end()) {
        sdk_msg = msg_manager->msg_cache_[db_msg->client_msg_id];
    } else {
        sdk_msg = std::make_shared<model::MessageModel>();
    }
    
    // Status and flags
    sdk_msg->status_ = db_msg->status;
    
    sdk_msg->is_pinned_ = db_msg->is_pinned;
    
    sdk_msg->is_deleted_ = db_msg->is_deleted;
    
    sdk_msg->is_recalled_ = db_msg->is_recalled;
    
    sdk_msg->is_group_msg_ = db_msg->is_group_msg;
    
    // Message content
    sdk_msg->content_ = db_msg->content;
    
    // User IDs
    sdk_msg->to_user_id_ = db_msg->to_user_id;
    
    sdk_msg->from_user_id_ = db_msg->from_user_id;
    
    // Message IDs
    sdk_msg->client_msg_id_ = db_msg->client_msg_id;
    
    sdk_msg->server_msg_id_ = db_msg->server_msg_id;
    
    // Conversation ID
    sdk_msg->conversation_id_ = db_msg->conversation_id;
    
    // Order indices
    sdk_msg->client_order_index_ = db_msg->client_order_index;
    
    sdk_msg->server_order_index_ = db_msg->server_order_index;
    
    // Timestamps
    sdk_msg->client_send_time_ = db_msg->client_send_time;
    
    sdk_msg->server_send_time_ = db_msg->server_send_time;
    
    // Extensions - TODO: Convert string to unordered_map
    // sdk_msg->sync_ext_ = parse_ext_string(db_msg->sync_ext);
    // sdk_msg->local_ext_ = parse_ext_string(db_msg->local_ext);
    
    return sdk_msg;
}

} // namespace roc::imsdk::core::message
