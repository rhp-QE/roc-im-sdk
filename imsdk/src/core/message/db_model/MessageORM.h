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
    
    std::string content;
    
    std::string to_user_id;
    
    std::string from_user_id;
    
    std::string client_msg_id;
    
    std::string server_msg_id;
    
    std::string conversation_id;
    
    int64_t client_order_index;
    
    int64_t server_order_index;
    
    double send_time;
    
    std::string sync_ext;
    
    std::string local_ext;

    std::string propertys;

    WCDB_CPP_ORM_DECLARATION(MessageORM)
};

} // namespace roc::imsdk::core::message

#endif