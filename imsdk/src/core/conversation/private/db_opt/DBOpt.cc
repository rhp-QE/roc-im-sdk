#include "DBOpt.h"

#include "WCDB/Expression.hpp"
#include "WCDB/Field.hpp"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include "WCDB/WCDBCpp.h"

namespace roc::imsdk::core::conversation {

static const std::string ConversationTableName = "conversation_table";

std::string DBOpt::table_name(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "default_conversation_table");
    return core::util::key_for_user(sdk_root->config().user_id, ConversationTableName);
}

bool DBOpt::create_conversation_table_if_need(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::conversation::ConversationORM>(table_name(CONTEXT_V));
}

bool DBOpt::insert_conversation(CONTEXT_T, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &conversation : conversations) {
            ret &= database->insertOrReplaceObject<core::conversation::ConversationORM>(*conversation, table_name(CONTEXT_V));
        }
        return ret;
    });
}

std::vector<std::shared_ptr<model::ConversationModel>> DBOpt::query_conversations(CONTEXT_T, int64_t cursor, int64_t limit, bool forward) {
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
        table_name(CONTEXT_V),
        where,
        WCDB_FIELD(core::conversation::ConversationORM::last_message_server_id).asOrder(WCDB::Order::DESC),
        WCDB::Expression(limit),
        WCDB::Expression()
    );

    if (!result.hasValue()) {
        return {};
    }

    std::vector<std::shared_ptr<model::ConversationModel>> convs;
    for (auto &item : result.value()) {
        convs.push_back(core::conversation::Convert::convert_db_conv_to_sdk_conv(CONTEXT_V, &item));
    }

    return convs;
}

bool DBOpt::update_conversations_status(CONTEXT_T, const std::vector<std::string> &conv_ids, int status) {
    // TODO: 实现批量更新会话状态
    return false;
}

std::shared_ptr<model::ConversationModel> DBOpt::conversation_for_id(CONTEXT_T, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, nullptr);

    auto result = database->getAllObjects<core::conversation::ConversationORM>(
        table_name(CONTEXT_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id,
        WCDB::Expression(),
        WCDB::Expression(),
        WCDB::Expression()
    );

    if (!result.hasValue() || result.value().empty()) {
        return nullptr;
    }

    return core::conversation::Convert::convert_db_conv_to_sdk_conv(CONTEXT_V, &result.value()[0]);
}

std::vector<std::shared_ptr<core::conversation::ConversationORM>> DBOpt::query_conversations_for_user(CONTEXT_T, int64_t cursor, int64_t limit) {
    // TODO: 实现查询用户会话列表
    return {};
}

bool DBOpt::delete_conversation(CONTEXT_T, const std::string &conv_id) {
    // TODO: 实现删除会话
    return false;
}

bool DBOpt::set_conversation_top(CONTEXT_T, const std::string &conv_id, bool is_top) {
    // TODO: 实现设置会话置顶
    return false;
}

bool DBOpt::set_conversation_mute(CONTEXT_T, const std::string &conv_id, bool is_mute) {
    // TODO: 实现设置会话免打扰
    return false;
}

} // namespace roc::imsdk::core::conversation
