#include "DBOpt.h"

#include "WCDB/Expression.hpp"
#include "WCDB/Field.hpp"
#include "core/common/logger_macro.h"
#include "core/common/macro.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

#include "WCDB/WCDBCpp.h"
#include <string>

namespace roc::imsdk::core::conversation {

static const std::string ConversationTableName = "conversation_table";

DBOpt::DBOpt(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

std::string DBOpt::p_TableName(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "default_conversation_table");
    return core::util::key_for_user(sdk_root->config().user_id, ConversationTableName);
}

bool DBOpt::CreateConversationTableIfNeed(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::conversation::ConversationORM>(p_TableName(CTX_V));
}

bool DBOpt::InsertConversation(CTX_T, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &conversation : conversations) {
            ret &= database->insertOrReplaceObject<core::conversation::ConversationORM>(*conversation, p_TableName(CTX_V));
        }
        return ret;
    });
}

std::vector<std::shared_ptr<model::ConversationModel>> DBOpt::QueryConversations(CTX_T, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, {});

    auto where = WCDB::Expression();

    if (cursor >= 0) {
        if (forward) {
            where = where && WCDB_FIELD(core::conversation::ConversationORM::last_message_time) < cursor;
        } else {
            where = where && WCDB_FIELD(core::conversation::ConversationORM::last_message_time) > cursor;
        }
    }

    auto result = database->getAllObjects<core::conversation::ConversationORM>(
        p_TableName(CTX_V),
        where,
        WCDB_FIELD(core::conversation::ConversationORM::last_message_server_id).asOrder(WCDB::Order::DESC),
        WCDB::Expression(limit),
        WCDB::Expression()
    );

    if (!result.hasValue()) {
        return {};
    }

    auto conv_manager = sdk_root->ConversationManager();
    std::vector<std::shared_ptr<model::ConversationModel>> convs;
    for (auto &item : result.value()) {
        convs.push_back(conv_manager->convert->ConvertDbConvToSdkConv(CTX_V, &item));
    }

    return convs;
}

bool DBOpt::UpdateConversationsStatus(CTX_T, const std::vector<std::string> &conv_ids, int status) {
    // TODO: 实现批量更新会话状态
    return false;
}

std::shared_ptr<model::ConversationModel> DBOpt::ConversationForId(CTX_T, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, nullptr);

    auto result = database->getAllObjects<core::conversation::ConversationORM>(
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id,
        WCDB::Expression(),
        WCDB::Expression(),
        WCDB::Expression()
    );

    if (!result.hasValue() || result.value().empty()) {
        return nullptr;
    }

    auto conv_manager = sdk_root->ConversationManager();
    return conv_manager->convert->ConvertDbConvToSdkConv(CTX_V, &result.value()[0]);
}

std::vector<std::shared_ptr<core::conversation::ConversationORM>> DBOpt::QueryConversationsForUser(CTX_T, int64_t cursor, int64_t limit) {
    // TODO: 实现查询用户会话列表
    return {};
}

bool DBOpt::DeleteConversation(CTX_T, const std::string &conv_id) {
    // TODO: 实现删除会话
    return false;
}

bool DBOpt::SetConversationTop(CTX_T, const std::string &conv_id, bool is_top) {
    // TODO: 实现设置会话置顶
    return false;
}

bool DBOpt::SetConversationMute(CTX_T, const std::string &conv_id, bool is_mute) {
    // TODO: 实现设置会话免打扰
    return false;
}

int64_t DBOpt::ChatsCursor(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 0)

    auto mmkv = sdk_root->mmkv();
    std::string key = sdk_root->config().user_id + "chats_cursor:";
    int64_t cursor = mmkv->getInt64(key, 0);

    LOG_INFO("ConvDBOpt", "get chats cursor: {}", cursor);

    return cursor;
}

void DBOpt::SetChatsCursor(CTX_T, int64_t cursor) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto mmkv = sdk_root->mmkv();
    std::string key = sdk_root->config().user_id + "chats_cursor";
    mmkv->set(cursor, key);
    LOG_INFO("ConvDBOpt", "set chats cursor: {}", cursor);
}

} // namespace roc::imsdk::core::conversation
