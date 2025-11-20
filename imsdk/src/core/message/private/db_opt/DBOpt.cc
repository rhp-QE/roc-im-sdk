#include "DBOpt.h"

#include "WCDB/CPPORMMacro.h"
#include "WCDB/Field.hpp"
#include "WCDB/StatementInsert.hpp"
#include "WCDB/Upsert.hpp"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/util.h"
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

//-----------------------
std::string DBOpt::p_TableName(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "defaule_message_table");
    return core::util::key_for_user(sdk_root->config().user_id, MessageTableName);
}

std::string DBOpt::p_MessageRangeKey(CONTEXT_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, MessageRangeKey);
    return core::util::key_for_user(sdk_root->config().user_id, MessageRangeKey + "_" + conv_id);
}

std::string DBOpt::p_OrderIndexKey(CONTEXT_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, OrderIndexKey);
    return core::util::key_for_user(sdk_root->config().user_id, OrderIndexKey + "_" + conv_id);
}
//-----------------------

bool DBOpt::CreateMessageTableIfNeed(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::message::MessageORM>(p_TableName(CONTEXT_V));
}

bool DBOpt::InsertOrReplaceMessage(CONTEXT_T, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertOrReplaceObject<core::message::MessageORM>(*message, p_TableName(CONTEXT_V));
        }
        return ret;
    });
}

bool DBOpt::InsertOrUpdateMessage(
    CONTEXT_T, 
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
                .insertIntoTable(p_TableName(CONTEXT_V))
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

bool DBOpt::SaveMessageRange(CONTEXT_T, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id) {
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

    LOG_INFO("MsgManager", "saveMessageRange, conv_id: {}, ranges: {}", conv_id, json_str);

    return mmkv->set(json_str, p_MessageRangeKey(CONTEXT_V, conv_id));
}


std::vector<std::pair<int64_t, int64_t>> DBOpt::MessageRange(CONTEXT_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, {});

    std::string json_str;
    bool res = mmkv->getString(p_MessageRangeKey(CONTEXT_V, conv_id), json_str);
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

    LOG_INFO("DBOpt", "load messages range from db, cid = {}, ranges = {}", conv_id, json_str);

    return ranges;
}

/// 获取消息 (直接从DB 中取)
std::shared_ptr<model::MessageModel> DBOpt::MessageForId(CONTEXT_T, std::string msg_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, nullptr);

    auto result = database->getAllObjects<core::message::MessageORM>(
        p_TableName(CONTEXT_V), 
        WCDB::Field(&core::message::MessageORM::client_msg_id) == msg_id
    );

    if (result.hasValue() && !result.value().empty()) {
        return core::message::Convert::ConvertDbMsgToSdkMsgTmp(CONTEXT_V, &(result.value().front()));
    }

    return nullptr;
}

std::vector<std::shared_ptr<model::MessageModel>> DBOpt::QueryMessagesForConvId(CONTEXT_T, std::string conv_id, int64_t cursor, int64_t limit, bool forward) {
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
        p_TableName(CONTEXT_V), 
        condition,
        WCDB_FIELD(core::message::MessageORM::client_order_index).asOrder(WCDB::Order::DESC),
        WCDB::Expression(limit),
        WCDB::Expression()
    );

    if (!result.hasValue()) {
        return {};
    }

    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs;
    for (auto &msg : result.value()) {
        sdk_msgs.push_back(core::message::Convert::ConvertDbMsgToSdkMsgTmp(CONTEXT_V, &msg));
    }

    return sdk_msgs;
}


/// 查询消息并设置优选使用的本地字段
void DBOpt::MessageMergeWithLocal(CONTEXT_T, std::string msg_id, message::MessageORM *db_msg_new) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VOID(database);

    auto resultFields = WCDB::ResultFields({
        WCDB_FIELD(message::MessageORM::local_ext),
        WCDB_FIELD(message::MessageORM::client_order_index)
    });

    auto result = database->getFirstObjectWithFields<core::message::MessageORM>(
        p_TableName(CONTEXT_V), 
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

void DBOpt::SetMsgOrderInConv(CONTEXT_T, std::string conv_id, int64_t order) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VOID(mmkv);

    auto msg_manager = sdk_root->MessageManager();

    // lock
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t old_max_order = mmkv->getInt64(p_OrderIndexKey(CONTEXT_V, conv_id), 0);
    if (order > old_max_order) {
        // std::cout<<"setMsgOrderInConv: "<<conv_id<<" : "<<order<<std::endl;
        mmkv->set(order, p_OrderIndexKey(CONTEXT_V, conv_id));
    }
}

int64_t DBOpt::NextMsgOrderInConv(CONTEXT_T, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 1);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, 1);

    auto msg_manager = sdk_root->MessageManager();

    // lock
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t max_order = mmkv->getInt64(p_OrderIndexKey(CONTEXT_V, conv_id), 0);
    mmkv->set(max_order + 1, p_OrderIndexKey(CONTEXT_V, conv_id));
    // std::cout<<"max_msg_order_in_conv: "<<conv_id<<" : "<<max_order<<std::endl;
    return max_order + 1;
}

} // namespace roc::imsdk::core::message
