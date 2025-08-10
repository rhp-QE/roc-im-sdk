#ifndef __MESSAGE_ORM_H__
#define __MESSAGE_ORM_H__

#include <WCDB/WCDBCpp.h>
#include <unordered_map>

namespace roc::imsdk::core::message {

class MessageORM {
public:
    MessageORM();
    ~MessageORM();
    
    // Status and flags
    int status;
    
    bool is_pinned;
    
    bool is_deleted;
    
    bool is_recalled;
    
    bool is_group_msg;
    
    // Message content
    std::string content;
    
    // User IDs
    std::string to_user_id;
    
    std::string from_user_id;
    
    // Message IDs
    std::string client_msg_id;
    
    std::string server_msg_id;
    
    // Conversation ID
    std::string conversation_id;
    
    // Order indices
    int64_t client_order_index;
    
    int64_t server_order_index;
    
    // Timestamps
    int64_t client_send_time;
    
    int64_t server_send_time;
    
    // Extensions
    std::string sync_ext;
    
    std::string local_ext;

    WCDB_CPP_ORM_DECLARATION(MessageORM)
};

} // namespace roc::imsdk::core::message

#endif