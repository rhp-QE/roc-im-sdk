#ifndef __MESSAGE_OPERATOR_H__
#define __MESSAGE_OPERATOR_H__

#include "WCDB/Database.hpp"
#include "imsdk/src/core/db/model/ConversationORM.h"
#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/macro.h"
#include <WCDB/WCDBCpp.h>
#include <memory>
#include <openssl/ec.h>

namespace roc::imsdk::db::operate {

static const std::string MessageTableName      = "messgae_table";
static const std::string ConversationTableName = "conversation_table";

// interface ------------------------------------------------------------------------------------------------------------------------

// 插入一条消息
inline bool insert_messgae(WCDB::Database *database, std::vector<db::MessageORM *>messages);

// 插入一条会话
inline bool insert_conversation(WCDB::Database *database, std::vector<db::ConversationORM *>conversations);

// 查找消息
inline std::unique_ptr<db::MessageORM> query_message_when_message_id(WCDB::Database *database, std::string message_id);

// 查找会话
inline std::unique_ptr<db::ConversationORM> query_conversation_when_conv_id(WCDB::Database *database, std::string conv_id);

// 删除消息
inline bool delete_message_when_message_id(WCDB::Database *database, std::vector<std::string> message_ids);

// 删除会话
inline bool delete_conversation_when_conv_id(WCDB::Database *database, std::vector<std::string> conv_ids);

// 更新消息
inline bool update_message_when_message_id(WCDB::Database *database, std::vector<db::MessageORM *>messages);

// 更新会话
inline bool update_conversation_when_conv_id(WCDB::Database *database, std::vector<db::ConversationORM *>conversations);

// 查询消息
// condition: 会话内的最新 limit 条消息
inline std::vector<std::unique_ptr<db::MessageORM>> query_message_when_conv_id(WCDB::Database *database, std::string conv_id, int64_t limit);

// 查询会话
// condition: 用户最新 limit 条会话
inline std::vector<std::unique_ptr<db::ConversationORM>> query_conversation(WCDB::Database *database, int64_t limit);

//--------------------------------------------------------------------------------------------------------------------------------------

// implementation -------------------------------------------------------------------------------------------------------------------
inline bool insert_messgae(WCDB::Database *database, std::vector<db::MessageORM *> messages) {
    CHECK_POINTER_OR_RETURN_VALUE(database, false)

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertObjects<db::MessageORM>(*message, MessageTableName);
        }
        return ret;
    });
}

inline bool insert_conversation(WCDB::Database *database, std::vector<db::ConversationORM *>conversations) {
    CHECK_POINTER_OR_RETURN_VALUE(database, false)

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &conversation : conversations) {
            ret &= database->insertObjects<db::ConversationORM>(*conversation, ConversationTableName);
        }   
        return ret;
    });
}

inline std::unique_ptr<db::MessageORM> query_message_when_message_id(WCDB::Database *database, std::string message_id) {
    return nullptr;
}

inline std::unique_ptr<db::ConversationORM> query_conversation_when_conv_id(WCDB::Database *database, std::string conv_id) {
    return nullptr;
}

inline bool delete_message_when_message_id(WCDB::Database *database, std::vector<std::string> message_ids) {
    return true;
}

inline bool delete_conversation_when_conv_id(WCDB::Database *database, std::vector<std::string> conv_ids) {
    return true;
}

inline bool update_message_when_message_id(WCDB::Database *database, std::vector<db::MessageORM *>messages) {
    return true;
}

inline bool update_conversation_when_conv_id(WCDB::Database *database, std::vector<db::ConversationORM *>conversations) {
    return true;
}

inline std::vector<std::unique_ptr<db::MessageORM>> query_message_when_conv_id(WCDB::Database *database, std::string conv_id, int64_t limit) {
    return {};
}

inline std::vector<std::unique_ptr<db::ConversationORM>> query_conversation(WCDB::Database *database, int64_t limit) {   
    return {};
}

} // namespace roc::imsdk::db::operate

#endif