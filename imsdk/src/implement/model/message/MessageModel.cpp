#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::model {

MessageModel::MessageModel(const std::string &content, const std::string &from_user_id, const std::string &to_user_id, const std::string &conversation_id)
    : content_(content), 
      from_user_id_(from_user_id),
      to_user_id_(to_user_id),
      conversation_id_(conversation_id)
{
}

std::string MessageModel::content() {
    return content_;
}

std::string MessageModel::to_user_id() {
    return to_user_id_;
}

std::string MessageModel::from_user_id() {
    return from_user_id_;
}

std::string MessageModel::conversation_id() {
    return conversation_id_;
}

} // namespace roc::imsdk::model 