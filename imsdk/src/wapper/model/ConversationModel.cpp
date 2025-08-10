#include "imsdk/src/include/model/conversation/ConversationModel.h"

namespace roc::imsdk::model {

std::string ConversationModel::name() {
    return name_;
}

int ConversationModel::unread_count() {
    return unread_count_;
}

std::string ConversationModel::avatar() {
    return avatar_url_;
}

ConvType ConversationModel::type() {
    return type_;
}

int64_t ConversationModel::last_update_time() {
    return last_update_time_;
}

std::string ConversationModel::last_message_id() {
    return last_message_id_;
}

std::string ConversationModel::conversation_id() {
    return conversation_id_;
}

std::shared_ptr<MessageModel> ConversationModel::last_message() {
    return last_message_;
}

// Additional accessor methods
std::string ConversationModel::avatar_url() {
    return avatar_url_;
}

int64_t ConversationModel::last_message_time() {
    return last_message_time_;
}

std::string ConversationModel::last_message_client_id() {
    return last_message_client_id_;
}

std::string ConversationModel::last_message_server_id() {
    return last_message_server_id_;
}

bool ConversationModel::is_top() {
    return is_top_;
}

int64_t ConversationModel::mask() {
    return mask_;
}

bool ConversationModel::is_muted() {
    return is_muted_;
}

bool ConversationModel::is_deleted() {
    return is_deleted_;
}

bool ConversationModel::is_blocked() {
    return is_blocked_;
}

std::string ConversationModel::draft() {
    return draft_;
}

std::unordered_map<std::string, std::string> ConversationModel::sync_ext() {
    return sync_ext_;
}

std::unordered_map<std::string, std::string> ConversationModel::local_ext() {
    return local_ext_;
}

} // namespace roc::imsdk::model 