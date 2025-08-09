#include "imsdk/src/include/model/conversation/ConversationModel.h"

namespace roc::imsdk::model {

std::string ConversationModel::conversation_id() const {
    return conversation_id_;
}

ConvType ConversationModel::type() const {
    return type_;
}

std::string ConversationModel::name() const {
    return name_;
}

std::string ConversationModel::avatar() const {
    return avatar_;
}

std::string ConversationModel::last_message_id() const {
    return last_message_id_;
}

int ConversationModel::unread_count() const {
    return unread_count_;
}

int64_t ConversationModel::last_update_time() {
    return last_update_time_;
}

} // namespace roc::imsdk::model 