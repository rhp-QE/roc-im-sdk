#ifndef __CONVERSATION_ORM_H__
#define __CONVERSATION_ORM_H__

#include <WCDB/WCDBCpp.h>
#include <cstdint>
#include <string>

namespace roc::imsdk::core::conversation {

class ConversationORM {
public:
    ConversationORM();
    ~ConversationORM();

    // Basic conversation info
    int type;
    
    std::string name;
    
    int unread_count;
    
    std::string avatar_url;
    
    int64_t last_message_time;
    
    std::string conversation_id;
    
    std::string last_message_client_id;
    
    std::string last_message_server_id;
    
    // Conversation settings
    bool is_top;
    
    int64_t mask;
    
    bool is_muted;
    
    bool is_deleted;
    
    bool is_blocked;
    
    std::string draft;
    
    // Extensions
    std::string sync_ext;
    
    std::string local_ext;

    WCDB_CPP_ORM_DECLARATION(ConversationORM)
};

} // namespace roc::imsdk::core::conversation

#endif