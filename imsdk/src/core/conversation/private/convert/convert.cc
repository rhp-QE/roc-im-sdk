#include "convert.h"

#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/common/util.h"
#include <boost/json.hpp>
#include <unordered_map>
#include <vector>

namespace roc::imsdk::core::conversation {

namespace {

std::vector<std::string> parseMembersJson(const std::string& members_json) {
    std::vector<std::string> members;
    if (members_json.empty()) {
        return members;
    }

    try {
        auto json_value = boost::json::parse(members_json);
        if (!json_value.is_array()) {
            return members;
        }

        const auto& arr = json_value.as_array();
        members.reserve(arr.size());
        for (const auto& item : arr) {
            if (item.is_string()) {
                members.emplace_back(std::string(item.as_string()));
            }
        }
    } catch (const std::exception&) {
        // ignore malformed json
    }

    return members;
}

} // namespace

Convert::Convert(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

/// 会话转换 网络会话 -> db 会话
std::shared_ptr<core::conversation::ConversationORM> Convert::ConvertNetConvToDbConv(const network::ConversationInfo *conv) {
    if (!conv) {
        return nullptr;
    }
    std::shared_ptr<core::conversation::ConversationORM> db_conv = std::make_shared<core::conversation::ConversationORM>();
    
    // Basic conversation info
    db_conv->type = conv->convtype();
    
    db_conv->name = conv->convname();
    
    db_conv->unread_count = conv->convunreadcount();
    
    db_conv->avatar_url = conv->convavatar();
    
    db_conv->last_message_time = conv->has_lastmsg() ? conv->lastmsg().sendtime() : 0;
    
    db_conv->conversation_id = conv->convid();
    
    db_conv->last_message_client_id = conv->has_lastmsg() ? conv->lastmsg().clientmsgid() : "";
    
    db_conv->last_message_server_id = conv->has_lastmsg() ? conv->lastmsg().servermsgid() : "";

    db_conv->members_json = conv->members();
    
    // Conversation settings
    db_conv->is_top = conv->istop();
    
    db_conv->mask = 0; // TODO: Set from network message if available
    
    db_conv->is_muted = conv->ismuted();
    
    db_conv->is_deleted = conv->isdelete();
    
    db_conv->is_blocked = conv->isblocked();
    
    db_conv->draft = ""; // TODO: Set from network message if available
    
    // Extensions
    db_conv->sync_ext = conv->syncext();
    
    db_conv->local_ext = ""; // TODO: Set from network message if available
    
    return db_conv;
}

/// 会话转换 db 会话 -> sdk 会话
std::shared_ptr<model::ConversationModel> Convert::ConvertDbConvToSdkConv(CTX_T, const core::conversation::ConversationORM *db_conv) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    if (!db_conv) {
        return nullptr;
    }

    std::shared_ptr<model::ConversationModel> sdk_conv = std::make_shared<model::ConversationModel>();
    
    // Basic conversation info
    sdk_conv->type_ = static_cast<model::ConvType>(db_conv->type);
    
    sdk_conv->name_ = db_conv->name;
    
    sdk_conv->unread_count_ = db_conv->unread_count;
    
    sdk_conv->avatar_url_ = db_conv->avatar_url;
    
    sdk_conv->last_message_time_ = db_conv->last_message_time;
    
    sdk_conv->conversation_id_ = db_conv->conversation_id;
    
    sdk_conv->last_message_client_id_ = db_conv->last_message_client_id;
    
    sdk_conv->last_message_server_id_ = db_conv->last_message_server_id;
    
    // Note: last_message_ needs to be set from elsewhere, setting to nullptr for now
    sdk_conv->last_message_ = nullptr;

    /// 会话成员
    sdk_conv->members_ = parseMembersJson(db_conv->members_json);
    
    // Set fields that don't exist in ConversationORM with default values
    sdk_conv->last_update_time_ = db_conv->last_message_time; // Use last_message_time as fallback
    
    sdk_conv->last_message_id_ = db_conv->last_message_server_id; // Use last_message_server_id as fallback
    
    // Conversation settings
    sdk_conv->is_top_ = db_conv->is_top;
    
    sdk_conv->mask_ = db_conv->mask;
    
    sdk_conv->is_muted_ = db_conv->is_muted;
    
    sdk_conv->is_deleted_ = db_conv->is_deleted;
    
    sdk_conv->is_blocked_ = db_conv->is_blocked;
    
    sdk_conv->draft_ = db_conv->draft;
    
    // Extensions
    auto sync_ext_result = util::MapParseFromString(db_conv->sync_ext);
    if (sync_ext_result) {
        sdk_conv->sync_ext_ = sync_ext_result.value();
    }
    auto local_ext_result = util::MapParseFromString(db_conv->local_ext);
    if (local_ext_result) {
        sdk_conv->local_ext_ = local_ext_result.value();
    }
    
    return sdk_conv;
}

} // namespace roc::imsdk::core::conversation
