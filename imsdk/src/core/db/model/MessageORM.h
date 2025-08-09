#ifndef __MESSAGE_ORM_H__
#define __MESSAGE_ORM_H__

#include <WCDB/WCDBCpp.h>

namespace roc::imsdk::db {

class MessageORM : public std::enable_shared_from_this<MessageORM> {
public:
    MessageORM();
    ~MessageORM();
    
    std::string server_msg_id;

    std::string client_msg_id;

    std::string server_index;

    std::string client_index;

    std::string conv_id;

    std::string sender_id;

    std::string content;

    long long send_time;

    bool read_time;

    std::string core_info;

    std::string ext;

    bool is_deleted;

    bool is_recalled;

    WCDB_CPP_ORM_DECLARATION(MessageORM)
};

}

#endif