#include "DBOpt.h"

#include "WCDB/CPPORMMacro.h"
#include "WCDB/Field.hpp"
#include "WCDB/StatementInsert.hpp"
#include "WCDB/Upsert.hpp"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/common/json_util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/convert/Convert.h"

#include "WCDB/WCDBCpp.h"
#include <boost/json/array.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <mutex>
#include <string>
#include <vector>

namespace roc::imsdk::core::message {

static const std::string MessageTableName = "messgae_table";
static const std::string MessageRangeKey = "messageRange";
static const std::string OrderIndexKey = "order_index";

DBOpt::DBOpt(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

//-----------------------
std::string DBOpt::p_TableName(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "defaule_message_table");
    return core::util::KeyForUser(sdk_root->config().user_id, MessageTableName);
}

std::string DBOpt::p_MessageRangeKey(CTX_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, MessageRangeKey);
    return core::util::KeyForUser(sdk_root->config().user_id, MessageRangeKey + "_" + conv_id);
}

std::string DBOpt::p_OrderIndexKey(CTX_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, OrderIndexKey);
    return core::util::KeyForUser(sdk_root->config().user_id, OrderIndexKey + "_" + conv_id);
}
//-----------------------

bool DBOpt::CreateMessageTableIfNeed(CTX_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::message::MessageORM>(p_TableName(CTX_V));
}

bool DBOpt::InsertOrReplaceMessage(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertOrReplaceObject<core::message::MessageORM>(*message, p_TableName(CTX_V));
        }
        return ret;
    });
}

bool DBOpt::InsertOrUpdateMessage(
    CTX_T, 
    std::vector<std::shared_ptr<core::message::MessageORM>> messages,
    const WCDB::Fields& fields,
    int type
) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 获取所有字段（用于插入）
    auto all_fields = MessageORM::allFields();

    // 构建需要更新的字段列表
    WCDB::Fields update_fields;
    if (type == 0) { // 白名单模式
        update_fields = fields;
    } else { // 黑名单模式
        update_fields = all_fields.fieldsByRemovingFields(fields);
    }
    
    // 获取主键字段（用于冲突检测）
    WCDB::Field primary_key = WCDB_FIELD(MessageORM::client_msg_id);

    return database->runTransaction([&](WCDB::Handle &handle) {
        for (auto &message : messages) {
            // 构建 UPSERT 子句：冲突时只更新部分字段
            WCDB::Upsert upsert = WCDB::Upsert()
                .onConflict()
                .indexed(primary_key)
                .doUpdate();
            
            // 为每个需要更新的字段设置 set().to()，使用绑定参数
            int param_index = all_fields.size() + 1; // 从所有字段之后开始
            for (const auto& field : update_fields) {
                upsert.set(field).to(WCDB::BindParameter(param_index++));
            }
            
            // 构建 INSERT 语句：插入所有字段
            WCDB::StatementInsert statement = WCDB::StatementInsert()
                .insertIntoTable(p_TableName(CTX_V))
                .columns(all_fields)
                .values(WCDB::BindParameter::bindParameters(all_fields.size()))
                .upsert(upsert);

            // 准备语句，失败则立即返回 false（事务回滚）
            if (!handle.prepare(statement)) {
                return false;
            }

            // 绑定所有字段的值（用于 INSERT）
            int index = 1;
            for (const auto& field : all_fields) {
                handle.bindObject(*message, field, index++);
            }

            // 绑定更新字段的值（用于 UPSERT 的 SET 子句）
            for (const auto& field : update_fields) {
                handle.bindObject(*message, field, index++);
            }

            // 执行语句，失败则立即返回 false（事务回滚）
            if (!handle.step()) {
                handle.finalize();
                return false;
            }
            handle.finalize();
        }
        return true;
    });
}

bool DBOpt::SaveMessageRange(CTX_T, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, false);

    /// 使用 boost::json 将 ranges 序列化成字符串
    boost::json::array json_ranges;
    for (const auto &range : ranges) {
        boost::json::array range_array;
        range_array.emplace_back(range.first);
        range_array.emplace_back(range.second);
        json_ranges.push_back(range_array);
    }

    std::string json_str = boost::json::serialize(json_ranges);

    LOG_INFO("MesageDBOpt", "save message range to db, cid: {}, ranges: {}", conv_id, json_str);

    return mmkv->set(json_str, p_MessageRangeKey(CTX_V, conv_id));
}


std::vector<std::pair<int64_t, int64_t>> DBOpt::MessageRange(CTX_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, {});

    std::string json_str;
    bool res = mmkv->getString(p_MessageRangeKey(CTX_V, conv_id), json_str);
    if (!res) {
        return {};
    }

    auto json_ranges = boost::json::parse(json_str);

    std::vector<std::pair<int64_t, int64_t>> ranges;
    for (const auto &range : json_ranges.as_array()) {
        if (!range.is_array()) continue;
        const auto& arr = range.as_array();
        if (arr.size() != 2) continue;
        int64_t first = arr[0].is_int64() ? arr[0].as_int64() : 0;
        int64_t second = arr[1].is_int64() ? arr[1].as_int64() : 0;
        ranges.emplace_back(first, second);
    }

    LOG_INFO("MessageDBOpt", "load messages range from db, cid = {}, ranges = {}", conv_id, json_str);

    return ranges;
}

/// 获取消息 (直接从DB 中取)
std::shared_ptr<model::MessageModel> DBOpt::MessageForId(CTX_T, std::string msg_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, nullptr);

    auto result = database->getAllObjects<core::message::MessageORM>(
        p_TableName(CTX_V), 
        WCDB::Field(&core::message::MessageORM::client_msg_id) == msg_id
    );

    if (result.hasValue() && !result.value().empty()) {
        auto msg_manager = sdk_root->MessageManager();
        return msg_manager->convert->ConvertDbMsgToSdkMsgTmp(CTX_V, &(result.value().front()));
    }

    return nullptr;
}

std::vector<std::shared_ptr<model::MessageModel>> DBOpt::QueryMessagesForConvId(CTX_T, std::string conv_id, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, {});

    auto condition = WCDB_FIELD(core::message::MessageORM::conversation_id) == conv_id;
    if (cursor > 0) {
        if (forward) {
            condition = condition && WCDB_FIELD(core::message::MessageORM::client_order_index) > cursor;
        } else {
            condition = condition && WCDB_FIELD(core::message::MessageORM::client_order_index) < cursor;
        }
    }

    auto result = database->getAllObjects<core::message::MessageORM>(
        p_TableName(CTX_V), 
        condition,
        WCDB_FIELD(core::message::MessageORM::client_order_index).asOrder(WCDB::Order::DESC),
        WCDB::Expression(limit),
        WCDB::Expression()
    );

    if (!result.hasValue()) {
        return {};
    }

    auto msg_manager = sdk_root->MessageManager();
    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs;
    for (auto &msg : result.value()) {
        sdk_msgs.push_back(msg_manager->convert->ConvertDbMsgToSdkMsgTmp(CTX_V, &msg));
    }

    return sdk_msgs;
}


/// 查询消息并设置优选使用的本地字段
void DBOpt::MessageMergeWithLocal(CTX_T, std::string msg_id, message::MessageORM *db_msg_new) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VOID(database);

    auto resultFields = WCDB::ResultFields({
        WCDB_FIELD(message::MessageORM::local_ext),
        WCDB_FIELD(message::MessageORM::client_order_index)
    });

    auto result = database->getFirstObjectWithFields<core::message::MessageORM>(
        p_TableName(CTX_V), 
        resultFields,
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );

    if (!result.hasValue()) {
        return;
    }

    auto db_msg_old = &(result.value());

    // --------------------------------------------------
    if (db_msg_old->client_order_index > 0) {
        db_msg_new->client_order_index = db_msg_old->client_order_index;
    }

    db_msg_new->local_ext = db_msg_old->local_ext;
    // --------------------------------------------------
}

void DBOpt::SetMsgOrderInConv(CTX_T, std::string conv_id, int64_t order) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto *mmkv = sdk_root->mmkv();
    auto msg_manager = sdk_root->MessageManager();

    // lock
    // TODO 移出去
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t old_max_order = mmkv->getInt64(p_OrderIndexKey(CTX_V, conv_id), 0);
    if (order > old_max_order) {
        LOG_INFO("MessageDBOpt", "update max message order index = {}, in cid ={}", order, conv_id)
        mmkv->set(order, p_OrderIndexKey(CTX_V, conv_id));
    }
}

int64_t DBOpt::NextMsgOrderInConv(CTX_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 1);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, 1);

    auto msg_manager = sdk_root->MessageManager();

    // lock
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t max_order = mmkv->getInt64(p_OrderIndexKey(CTX_V, conv_id), 0);
    mmkv->set(max_order + 1, p_OrderIndexKey(CTX_V, conv_id));
    LOG_INFO("MessageDBOpt", "get next message order index = {}, in cid = {}", max_order + 1, conv_id)
    
    return max_order + 1;
}

bool DBOpt::SetMessagePin(CTX_T, const std::string &msg_id, bool is_pinned) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    core::message::MessageORM obj;
    obj.is_pinned = is_pinned;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::is_pinned)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessagePin msg_id: {}, is_pinned: {}, result: {}", msg_id, is_pinned, update_result);
    return update_result;
}

bool DBOpt::SetMessageSyncExt(CTX_T, const std::string &msg_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 1. 查询数据库获取当前的 sync_ext
    auto result = database->getAllObjects<core::message::MessageORM>(
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id,
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
            LOG_INFO("MsgDBOpt", "Failed to parse existing sync_ext: {}", parse_result.error().to_string());
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
        LOG_INFO("MsgDBOpt", "Failed to serialize sync_ext: {}", sync_ext_str_result.error().to_string());
        return false;
    }
    std::string sync_ext_str = sync_ext_str_result.value();

    // 5. 写回数据库
    core::message::MessageORM obj;
    obj.sync_ext = sync_ext_str;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::sync_ext)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessageSyncExt msg_id: {}, result: {}", msg_id, update_result);
    return update_result;
}

bool DBOpt::SetMessagePropertys(CTX_T, const std::string &msg_id, const std::vector<int32_t> &propertys) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 序列化 propertys 为 JSON string（整体替换，不合并）
    auto propertys_str_result = json_util::Int32VectorSerializeAsString(propertys);
    if (!propertys_str_result) {
        LOG_INFO("MsgDBOpt", "Failed to serialize propertys: {}", propertys_str_result.error().to_string());
        return false;
    }
    std::string propertys_str = propertys_str_result.value();

    // 写回数据库
    core::message::MessageORM obj;
    obj.propertys = propertys_str;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::propertys)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessagePropertys msg_id: {}, result: {}", msg_id, update_result);
    return update_result;
}

bool DBOpt::SetMessageLocalExt(CTX_T, const std::string &msg_id, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    // 1. 查询数据库获取当前的 local_ext
    auto result = database->getAllObjects<core::message::MessageORM>(
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id,
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
            LOG_INFO("MsgDBOpt", "Failed to parse existing local_ext: {}", parse_result.error().to_string());
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
        LOG_INFO("MsgDBOpt", "Failed to serialize local_ext: {}", local_ext_str_result.error().to_string());
        return false;
    }
    std::string local_ext_str = local_ext_str_result.value();

    // 5. 写回数据库
    core::message::MessageORM obj;
    obj.local_ext = local_ext_str;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::local_ext)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessageLocalExt msg_id: {}, result: {}", msg_id, update_result);
    return update_result;
}

bool DBOpt::SetMessageDeleted(CTX_T, const std::string &msg_id, bool is_deleted) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    core::message::MessageORM obj;
    obj.is_deleted = is_deleted;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::is_deleted)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessageDeleted msg_id: {}, is_deleted: {}, result: {}", msg_id, is_deleted, update_result);
    return update_result;
}

bool DBOpt::SetMessageRecalled(CTX_T, const std::string &msg_id, bool is_recalled) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);
    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    core::message::MessageORM obj;
    obj.is_recalled = is_recalled;
    WCDB::Fields fields = {WCDB_FIELD(core::message::MessageORM::is_recalled)};
    bool update_result = database->updateObject<core::message::MessageORM>(
        obj,
        fields,
        p_TableName(CTX_V),
        WCDB_FIELD(core::message::MessageORM::client_msg_id) == msg_id
    );
    LOG_INFO("MsgDBOpt", "SetMessageRecalled msg_id: {}, is_recalled: {}, result: {}", msg_id, is_recalled, update_result);
    return update_result;
}

} // namespace roc::imsdk::core::message
