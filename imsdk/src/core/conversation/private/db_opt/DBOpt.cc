#include "DBOpt.h"

#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include "WCDB/WCDBCpp.h"

namespace roc::imsdk::core::conversation {

static const std::string ConversationTableName = "conversation_table";

std::string DBOpt::table_name(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, "default_conversation_table");
    return core::util::key_for_user(sdk_root->config().user_id, ConversationTableName);
}

bool DBOpt::create_conversation_table_if_need(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->createTable<core::conversation::ConversationORM>(table_name(w_sdk_root));
}

bool DBOpt::insert_conversation(W_SDK_ROOT, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, false);

    auto database = sdk_root->database();
    CHECK_POINTER_OR_RETURN_VALUE(database, false);

    return database->runTransaction([&](WCDB::Handle &handle) {
        bool ret = true;
        for (auto &conversation : conversations) {
            ret &= database->insertObjects<core::conversation::ConversationORM>(*conversation, table_name(w_sdk_root));
        }
        return ret;
    });
}

bool DBOpt::update_conversations_status(W_SDK_ROOT, const std::vector<std::string> &conv_ids, int status) {
    // TODO: 实现批量更新会话状态
    return false;
}

std::shared_ptr<core::conversation::ConversationORM> DBOpt::query_conversation_by_id(W_SDK_ROOT, const std::string &conv_id) {
    // TODO: 实现根据ID查询会话
    return nullptr;
}

std::vector<std::shared_ptr<core::conversation::ConversationORM>> DBOpt::query_conversations_for_user(W_SDK_ROOT, int64_t cursor, int64_t limit) {
    // TODO: 实现查询用户会话列表
    return {};
}

bool DBOpt::delete_conversation(W_SDK_ROOT, const std::string &conv_id) {
    // TODO: 实现删除会话
    return false;
}

bool DBOpt::set_conversation_top(W_SDK_ROOT, const std::string &conv_id, bool is_top) {
    // TODO: 实现设置会话置顶
    return false;
}

bool DBOpt::set_conversation_mute(W_SDK_ROOT, const std::string &conv_id, bool is_mute) {
    // TODO: 实现设置会话免打扰
    return false;
}

} // namespace roc::imsdk::core::conversation
