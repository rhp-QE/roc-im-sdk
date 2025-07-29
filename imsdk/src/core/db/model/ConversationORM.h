#ifndef __CONVERSATION_ORM_H__
#define __CONVERSATION_ORM_H__

#include <WCDB/WCDBCpp.h>
#include <cstdint>

namespace roc::imsdk::db {

class ConversationORM {
public:
    ConversationORM();
    ~ConversationORM();

    std::string conv_id;

    std::string name;

    long long conv_type;

    std::string last_message_id;

    long long last_message_time;

    long long unread_count;

    std::string draft;

    bool is_top;

    bool is_muted;

    bool is_deleted;

    long long delete_time;

    bool is_blocked;

    std::string core_info; // sdk 专用

    std::string ext;

    long long mask;

    WCDB_CPP_ORM_DECLARATION(ConversationORM)
};

} // namespace roc::imsdk::db

#endif