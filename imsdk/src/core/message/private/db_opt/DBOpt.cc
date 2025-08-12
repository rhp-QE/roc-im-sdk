#include "DBOpt.h"

#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include "WCDB/WCDBCpp.h"

namespace roc::imsdk::core::message {

static const std::string MessageTableName = "messgae_table";

std::string DBOpt::tabel_name(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "defaule_message_table");
    return core::util::key_for_user(sdk_root->config().user_id, MessageTableName);
}

bool DBOpt::create_message_table_if_need(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::message::MessageORM>(tabel_name(w_sdk_root));
}

bool DBOpt::insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertObjects<core::message::MessageORM>(*message, tabel_name(w_sdk_root));
        }
        return ret;
    });
}

} // namespace roc::imsdk::core::message
