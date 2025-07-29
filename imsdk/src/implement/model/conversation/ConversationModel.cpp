#include "imsdk/src/include/model/conversation/ConversationModel.h"

namespace roc::imsdk::model {

std::string ConversationModel::get_conversation_id() const {
    return conversation_id_;
}

ConversationType ConversationModel::get_type() const {
    return type_;
}

std::string ConversationModel::get_name() const {
    return name_;
}

std::string ConversationModel::get_avatar() const {
    return avatar_;
}

std::string ConversationModel::get_last_message_id() const {
    return last_message_id_;
}

int ConversationModel::get_unread_count() const {
    return unread_count_;
}

} // namespace roc::imsdk::model 