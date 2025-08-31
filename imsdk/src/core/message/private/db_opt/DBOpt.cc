#include "DBOpt.h"

#include "WCDB/CPPORMMacro.h"
#include "WCDB/Field.hpp"
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
static const std::string MessageRangeKey = "message_range";
static const std::string OrderIndexKey = "order_index";

//-----------------------
std::string DBOpt::tabel_name(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "defaule_message_table");
    return core::util::key_for_user(sdk_root->config().user_id, MessageTableName);
}

std::string DBOpt::message_range_key(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, MessageRangeKey);
    return core::util::key_for_user(sdk_root->config().user_id, MessageRangeKey + "_" + conv_id);
}

std::string DBOpt::order_index_key(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, OrderIndexKey);
    return core::util::key_for_user(sdk_root->config().user_id, OrderIndexKey + "_" + conv_id);
}
//-----------------------

bool DBOpt::create_message_table_if_need(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::message::MessageORM>(tabel_name(w_sdk_root));
}

bool DBOpt::insert_or_replace_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertOrReplaceObject<core::message::MessageORM>(*message, tabel_name(w_sdk_root));
        }
        return ret;
    });
}

bool DBOpt::insert_or_replace_message_when_net(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    const std::unordered_set<std::string> not_update_field {"client_msg_id", "client_order_index", "client_send_time", "local_ext"};


    std::vector<WCDB::Field> update_fields;
    for (const auto& field : MessageORM::allFields()) {
        if (not_update_field.find(field.getDescription()) != not_update_field.end()) {
            continue; // 过滤掉不需要更新的元素
        }
        update_fields.push_back(field);
    }

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertOrReplaceObject<core::message::MessageORM>(*message, tabel_name(w_sdk_root), update_fields);
        }
        return ret;
    });
}

bool DBOpt::save_message_range(W_SDK_ROOT, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id) {
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

    LOG_INFO("DBOpt", "save_message_range, conv_id: {}, ranges: {}", conv_id, json_str);

    return mmkv->set(json_str, message_range_key(w_sdk_root, conv_id));
}


std::vector<std::pair<int64_t, int64_t>> DBOpt::message_range(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, {});

    std::string json_str;
    bool res = mmkv->getString(message_range_key(w_sdk_root, conv_id), json_str);
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

    return ranges;
}

/// 获取消息 (直接从DB 中取)
std::shared_ptr<model::MessageModel> DBOpt::message_for_id(W_SDK_ROOT, std::string msg_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, nullptr);

    auto result = database->getAllObjects<core::message::MessageORM>(
        tabel_name(w_sdk_root), 
        WCDB::Field(&core::message::MessageORM::client_msg_id) == msg_id
    );

    if (result.hasValue() && !result.value().empty()) {
        return core::message::Convert::convert_db_msg_to_sdk_msg(w_sdk_root, &(result.value().front()));
    }

    return nullptr;
}

std::vector<std::shared_ptr<model::MessageModel>> DBOpt::query_messages_for_conv_id(W_SDK_ROOT, std::string conv_id, int64_t cursor, int64_t limit, bool forward) {
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
        tabel_name(w_sdk_root), 
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
        sdk_msgs.push_back(core::message::Convert::convert_db_msg_to_sdk_msg(w_sdk_root, &msg));
    }

    return sdk_msgs;
}


/// 查询消息并设置优选使用的本地字段
void DBOpt::message_merge_with_local(W_SDK_ROOT, std::string msg_id, message::MessageORM *db_msg_new) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VOID(database);

    auto resultFields = WCDB::ResultFields({
        WCDB_FIELD(message::MessageORM::local_ext),
        WCDB_FIELD(message::MessageORM::client_order_index)
    });

    auto result = database->getFirstObjectWithFields<core::message::MessageORM>(
        tabel_name(w_sdk_root), 
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

void DBOpt::set_msg_order_in_conv(W_SDK_ROOT, std::string conv_id, int64_t order) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VOID(mmkv);

    auto msg_manager = sdk_root->message_manager();

    // lock
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t old_max_order = mmkv->getInt64(order_index_key(w_sdk_root, conv_id), 0);
    if (order > old_max_order) {
        // std::cout<<"set_msg_order_in_conv: "<<conv_id<<" : "<<order<<std::endl;
        mmkv->set(order, order_index_key(w_sdk_root, conv_id));
    }
}

int64_t DBOpt::next_msg_order_in_conv(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 1);

    auto *mmkv = sdk_root->mmkv();
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, 1);

    auto msg_manager = sdk_root->message_manager();

    // lock
    std::lock_guard<std::mutex> lock(msg_manager->msg_order_mutex_);

    int64_t max_order = mmkv->getInt64(order_index_key(w_sdk_root, conv_id), 0);
    mmkv->set(max_order + 1, order_index_key(w_sdk_root, conv_id));
    // std::cout<<"max_msg_order_in_conv: "<<conv_id<<" : "<<max_order<<std::endl;
    return max_order + 1;
}

} // namespace roc::imsdk::core::message
