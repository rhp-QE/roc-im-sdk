#ifndef ROC_IMSDK_MODEL_CONVERSATIONMODEL_H
#define ROC_IMSDK_MODEL_CONVERSATIONMODEL_H

#include <functional>
#include <string>
#include <vector>
#include <memory>


namespace roc::imsdk::core {
    class ConversationManager;
}

namespace roc::imsdk::core::conversation {
    class Convert;
}

namespace roc::imsdk::model {

class MessageModel;

enum class ConvType {
    Single,
    Group
};


class ConversationModel {
public:
    std::string name();
    int unread_count();
    std::string avatar();
    ConvType type();
    int64_t last_update_time();
    std::string last_message_id();
    std::string conversation_id();
    std::shared_ptr<MessageModel> last_message();

    friend class roc::imsdk::core::ConversationManager;
    friend class roc::imsdk::core::conversation::Convert;

private:
    ConvType type_;
    std::string name_;
    std::string avatar_;
    int unread_count_ = 0;
    int64_t last_update_time_;
    std::string last_message_id_;
    std::string conversation_id_;
    std::shared_ptr<MessageModel> last_message_;
};


enum class ConvUpdateReason {
    UPDATE,
    DELETE,
};


struct QueryUserConvsResult {
    std::vector<std::shared_ptr<const ConversationModel>> convs;
    int64_t cursor;
    bool has_more;
};


// callback -------------
using OnConvUpdateCallbackType = std::function<void(std::shared_ptr<const ConversationModel> conv, ConvUpdateReason reason)>;
// ------------------------

} // namespace roc::imsdk::model

#endif // ROC_IMSDK_MODEL_CONVERSATIONMODEL_H 