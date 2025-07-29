#ifndef ROC_IMSDK_MODEL_CONVERSATIONMODEL_H
#define ROC_IMSDK_MODEL_CONVERSATIONMODEL_H

#include <string>

namespace roc::imsdk::model {

enum class ConversationType {
    Single,
    Group
};

class ConversationModel {
public:
    // Getter methods
    std::string get_conversation_id() const;
    ConversationType get_type() const;
    std::string get_name() const;
    std::string get_avatar() const;
    std::string get_last_message_id() const;
    int get_unread_count() const;

private:
    std::string conversation_id_;
    ConversationType type_;
    std::string name_;
    std::string avatar_;
    std::string last_message_id_;
    int unread_count_ = 0;
};

} // namespace roc::imsdk::model

#endif // ROC_IMSDK_MODEL_CONVERSATIONMODEL_H 