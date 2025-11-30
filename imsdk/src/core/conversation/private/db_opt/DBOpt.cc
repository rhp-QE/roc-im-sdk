#include "DBOpt.h"

#include "WCDB/Expression.hpp"
#include "WCDB/Field.hpp"
#include "core/common/logger_macro.h"
#include "core/common/macro.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/common/json_util.h"
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
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    core::conversation::ConversationORM obj;
    obj.is_top = is_top;

    WCDB::Fields fields = {WCDB_FIELD(core::conversation::ConversationORM::is_top)};
    bool result = database->updateObject<core::conversation::ConversationORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );

    LOG_INFO("ConvDBOpt", "SetConversationTop conv_id: {}, is_top: {}, result: {}", conv_id, is_top, result);
    
    return result;
}

bool DBOpt::SetConversationMute(CTX_T, const std::string &conv_id, bool is_mute) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    core::conversation::ConversationORM obj;
    obj.is_muted = is_mute;

    WCDB::Fields fields = {WCDB_FIELD(core::conversation::ConversationORM::is_muted)};
    bool result = database->updateObject<core::conversation::ConversationORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );
    LOG_INFO("ConvDBOpt", "SetConversationMute conv_id: {}, is_mute: {}, result: {}", conv_id, is_mute, result);
    
    return result;
}

bool DBOpt::SetConversationBlock(CTX_T, const std::string &conv_id, bool is_block) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    core::conversation::ConversationORM obj;
    obj.is_blocked = is_block;

    WCDB::Fields fields = {WCDB_FIELD(core::conversation::ConversationORM::is_blocked)};
    bool result = database->updateObject<core::conversation::ConversationORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );
    LOG_INFO("ConvDBOpt", "SetConversationBlock conv_id: {}, is_block: {}, result: {}", conv_id, is_block, result);
    
    return result;
}

bool DBOpt::SetConversationSyncExt(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 1. 查询数据库获取当前的 sync_ext
    auto result = database->getAllObjects<core::conversation::ConversationORM>(
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id,
        WCDB::Expression(),
        WCDB::Expression(),
        WCDB::Expression()
    );

    // 2. 反序列化现有的 sync_ext string 为 map
    std::unordered_map<std::string, std::string> existing_sync_ext;
    if (result.hasValue() && !result.value().empty()) {
        std::string existing_sync_ext_str = result.value()[0].sync_ext;
        auto parse_result = json_util::MapParseFromString(existing_sync_ext_str);
        if (parse_result) {
            existing_sync_ext = parse_result.value();
        } else {
            // 解析失败，使用空 map
            LOG_INFO("ConvDBOpt", "Failed to parse existing sync_ext: {}", parse_result.error().to_string());
        }
    }

    // 3. 合并传入的 map 与现有 map
    std::unordered_map<std::string, std::string> merged_sync_ext = existing_sync_ext;
    for (const auto& [key, value] : sync_ext) {
        merged_sync_ext[key] = value;
    }

    // 4. 序列化合并后的 map 为 string
    auto sync_ext_str_result = json_util::MapSerializeAsString(merged_sync_ext);
    if (!sync_ext_str_result) {
        LOG_INFO("ConvDBOpt", "Failed to serialize sync_ext: {}", sync_ext_str_result.error().to_string());
        return false;
    }
    std::string sync_ext_str = sync_ext_str_result.value();

    // 5. 写回数据库
    core::conversation::ConversationORM obj;
    obj.sync_ext = sync_ext_str;

    WCDB::Fields fields = {WCDB_FIELD(core::conversation::ConversationORM::sync_ext)};
    bool update_result = database->updateObject<core::conversation::ConversationORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );
    LOG_INFO("ConvDBOpt", "SetConversationSyncExt conv_id: {}, result: {}", conv_id, update_result);
    
    return update_result;
}

bool DBOpt::SetConversationLocalExt(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 1. 查询数据库获取当前的 local_ext
    auto result = database->getAllObjects<core::conversation::ConversationORM>(
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id,
        WCDB::Expression(),
        WCDB::Expression(),
        WCDB::Expression()
    );

    // 2. 反序列化现有的 local_ext string 为 map
    std::unordered_map<std::string, std::string> existing_local_ext;
    if (result.hasValue() && !result.value().empty()) {
        std::string existing_local_ext_str = result.value()[0].local_ext;
        auto parse_result = json_util::MapParseFromString(existing_local_ext_str);
        if (parse_result) {
            existing_local_ext = parse_result.value();
        } else {
            // 解析失败，使用空 map
            LOG_INFO("ConvDBOpt", "Failed to parse existing local_ext: {}", parse_result.error().to_string());
        }
    }

    // 3. 合并传入的 map 与现有 map
    std::unordered_map<std::string, std::string> merged_local_ext = existing_local_ext;
    for (const auto& [key, value] : local_ext) {
        merged_local_ext[key] = value;
    }

    // 4. 序列化合并后的 map 为 string
    auto local_ext_str_result = json_util::MapSerializeAsString(merged_local_ext);
    if (!local_ext_str_result) {
        LOG_INFO("ConvDBOpt", "Failed to serialize local_ext: {}", local_ext_str_result.error().to_string());
        return false;
    }
    std::string local_ext_str = local_ext_str_result.value();

    // 5. 写回数据库
    core::conversation::ConversationORM obj;
    obj.local_ext = local_ext_str;

    WCDB::Fields fields = {WCDB_FIELD(core::conversation::ConversationORM::local_ext)};
    bool update_result = database->updateObject<core::conversation::ConversationORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );
    LOG_INFO("ConvDBOpt", "SetConversationLocalExt conv_id: {}, result: {}", conv_id, update_result);
    
    return update_result;
}

int64_t DBOpt::ChatsCursor(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 0)

    auto mmkv = sdk_root->mmkv();
    std::string key = sdk_root->config().user_id + "chats_cursor:";
    int64_t cursor = mmkv->getInt64(key, 0);

    LOG_INFO("ConvDBOpt", "get chats cursor: {}", cursor);

    return cursor;
}

/// 查询会话并合并本地独有字段
void DBOpt::ConversationMergeWithLocal(CTX_T, const std::string &conv_id, core::conversation::ConversationORM *db_conv_new) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VOID(database);

    auto resultFields = WCDB::ResultFields({
        WCDB_FIELD(core::conversation::ConversationORM::local_ext),
        WCDB_FIELD(core::conversation::ConversationORM::draft)
    });

    auto result = database->getFirstObjectWithFields<core::conversation::ConversationORM>(
        p_TableName(CTX_V), 
        resultFields,
        WCDB_FIELD(core::conversation::ConversationORM::conversation_id) == conv_id
    );

    if (!result.hasValue()) {
        return;
    }

    auto db_conv_old = &(result.value());

    // 合并本地独有字段
    db_conv_new->local_ext = db_conv_old->local_ext;
    db_conv_new->draft = db_conv_old->draft;
}

void DBOpt::SetChatsCursor(CTX_T, int64_t cursor) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto mmkv = sdk_root->mmkv();
    std::string key = sdk_root->config().user_id + "chats_cursor";
    mmkv->set(cursor, key);
    LOG_INFO("ConvDBOpt", "set chats cursor: {}", cursor);
}

} // namespace roc::imsdk::core::conversation
