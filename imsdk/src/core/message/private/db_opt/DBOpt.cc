#include "DBOpt.h"

#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include "WCDB/WCDBCpp.h"

namespace roc::imsdk::core::message {

static const std::string MessageTableName = "messgae_table";

bool DBOpt::insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    std::string table_name = core::util::key_for_user(sdk_root->config().user_id, MessageTableName);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &message : messages) {
            ret &= database->insertObjects<core::message::MessageORM>(*message, table_name);
        }
        return ret;
    });
}

} // namespace roc::imsdk::core::message
