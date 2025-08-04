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
    std::string conversation_id() const;
    ConversationType type() const;
    std::string name() const;
    std::string avatar() const;
    std::string last_message_id() const;
    int unread_count() const;
    int64_t last_update_time();

private:
    std::string conversation_id_;
    ConversationType type_;
    std::string name_;
    std::string avatar_;
    std::string last_message_id_;
    int64_t last_update_time_;
    int unread_count_ = 0;
};

} // namespace roc::imsdk::model

#endif // ROC_IMSDK_MODEL_CONVERSATIONMODEL_H 