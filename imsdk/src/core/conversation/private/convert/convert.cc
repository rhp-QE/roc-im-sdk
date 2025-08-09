#include "convert.h"

namespace roc::imsdk::core::conversation {

/// 会话转换 网络会话 -> db 会话
std::shared_ptr<core::conversation::ConversationORM> Convert::convert_net_conv_to_db_conv(const network::ConversationInfo *conv) {
    if (!conv) {
        return nullptr;
    }
    std::shared_ptr<core::conversation::ConversationORM> db_conv = std::make_shared<core::conversation::ConversationORM>();
    
    db_conv->conv_id = conv->convid();
    db_conv->name = conv->convname();
    db_conv->avatar = conv->convavatar();
    db_conv->conv_type = conv->convtype();
    db_conv->unread_count = conv->convunreadcount();
    db_conv->is_top = conv->istop();
    db_conv->is_muted = conv->ismuted();
    db_conv->is_deleted = conv->isdelete();
    db_conv->is_blocked = conv->isblocked();
    db_conv->ext = conv->ext();
    
    // 设置最后消息相关信息
    if (conv->has_lastmsg()) {
        db_conv->last_message_clent_id = conv->lastmsg().clientmsgid();
        db_conv->last_message_server_id = conv->lastmsg().servermsgid();
        db_conv->last_message_time = conv->lastmsg().sendtime();
    }
    
    return db_conv;
}

/// 会话转换 db 会话 -> sdk 会话
std::shared_ptr<model::ConversationModel> Convert::convert_db_conv_to_sdk_conv(const core::conversation::ConversationORM *db_conv) {
    if (!db_conv) {
        return nullptr;
    }
    std::shared_ptr<model::ConversationModel> sdk_conv = std::make_shared<model::ConversationModel>();
    
    // 设置私有成员（通过友元访问）
    sdk_conv->conversation_id_ = db_conv->conv_id;
    sdk_conv->name_ = db_conv->name;
    sdk_conv->avatar_ = db_conv->avatar;
    sdk_conv->unread_count_ = db_conv->unread_count;
    sdk_conv->last_update_time_ = db_conv->last_message_time;
    sdk_conv->last_message_id_ = db_conv->last_message_server_id;
    
    // 设置会话类型
    if (db_conv->conv_type == 1) { // 假设1表示群聊，0表示单聊
        sdk_conv->type_ = model::ConvType::Group;
    } else {
        sdk_conv->type_ = model::ConvType::Single;
    }
    
    // 注意：last_message_需要从其他地方获取，这里暂时设为nullptr
    sdk_conv->last_message_ = nullptr;
    
    return sdk_conv;
}

} // namespace roc::imsdk::core::conversation
