#include "DBOpt.h"

#include "WCDB/Field.hpp"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include "WCDB/WCDBCpp.h"
#include <boost/json/array.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <string>
#include <vector>

namespace roc::imsdk::core::message {

static const std::string MessageTableName = "messgae_table";
static const std::string MessageRangeKey = "message_range";

std::string DBOpt::tabel_name(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "defaule_message_table");
    return core::util::key_for_user(sdk_root->config().user_id, MessageTableName);
}

std::string DBOpt::message_range_key(W_SDK_ROOT, std::string conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, MessageRangeKey);
    return core::util::key_for_user(sdk_root->config().user_id, MessageRangeKey + "_" + conv_id);
}

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

} // namespace roc::imsdk::core::message
