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

    // Additional accessor methods
    std::string avatar_url();
    int64_t last_message_time();
    std::string last_message_client_id();
    std::string last_message_server_id();
    bool is_top();
    int64_t mask();
    bool is_muted();
    bool is_deleted();
    bool is_blocked();
    std::string draft();
    std::unordered_map<std::string, std::string> sync_ext();
    std::unordered_map<std::string, std::string> local_ext();

    friend class roc::imsdk::core::ConversationManager;
    friend class roc::imsdk::core::conversation::Convert;

private:
    ConvType type_;
    
    std::string name_;
    
    int unread_count_ = 0;
    
    std::string avatar_url_;
    
    int64_t last_message_time_;
    
    int64_t last_update_time_;
    
    std::string conversation_id_;
    
    std::string last_message_id_;
    
    std::string last_message_client_id_;
    
    std::string last_message_server_id_;
    
    std::shared_ptr<MessageModel> last_message_;

    bool is_top_;
    
    int64_t mask_;
    
    bool is_muted_;
    
    bool is_deleted_;
    
    bool is_blocked_;
    
    std::string draft_;

    std::unordered_map<std::string, std::string> sync_ext_;
    
    std::unordered_map<std::string, std::string> local_ext_;
};


enum class ConvUpdateReason : int {
    UPDATE = 0,
    DELETE = 1,
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