#include "imsdk/src/core/message/opt/db_opt/db_opt.h"

#include "imsdk/src/core/common/util.h"

namespace roc::imsdk::core::message::dbopt {

static const std::string MessageTableName = "messgae_table";

bool insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages) {
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

} // namespace roc::imsdk::core::message::dbopt