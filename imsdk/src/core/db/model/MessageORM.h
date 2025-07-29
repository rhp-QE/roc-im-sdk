#ifndef __MESSAGE_ORM_H__
#define __MESSAGE_ORM_H__

#include <WCDB/WCDBCpp.h>

namespace roc::imsdk::db {

class MessageORM {
public:
    MessageORM();
    ~MessageORM();
    
    std::string msg_id;

    std::string conv_id;

    std::string sender_id;

    std::string content;

    long long send_time;

    bool read_time;

    std::string core_info;

    std::string ext;

    WCDB_CPP_ORM_DECLARATION(MessageORM)
};

}

#endif