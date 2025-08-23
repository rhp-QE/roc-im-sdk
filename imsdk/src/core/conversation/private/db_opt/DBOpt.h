#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

namespace roc::imsdk::core::conversation {

class DBOpt {
public:
    /// 创建会话表（如果不存在）
    static bool create_conversation_table_if_need(W_SDK_ROOT);

    /// 插入会话到数据库
    static bool insert_conversation(W_SDK_ROOT, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations);

    /// 捞取会话
    static std::vector<std::shared_ptr<model::ConversationModel>> query_conversations(W_SDK_ROOT, int64_t cursor, int64_t limit, bool forward = true);

    /// 批量更新会话状态（如已读状态）
    static bool update_conversations_status(W_SDK_ROOT, const std::vector<std::string> &conv_ids, int status);

    /// 根据会话ID查询会话
    static std::shared_ptr<model::ConversationModel> query_conversation_by_id(W_SDK_ROOT, const std::string &conv_id);

    /// 查询用户的所有会话
    static std::vector<std::shared_ptr<core::conversation::ConversationORM>> query_conversations_for_user(W_SDK_ROOT, int64_t cursor, int64_t limit);

    /// 删除会话
    static bool delete_conversation(W_SDK_ROOT, const std::string &conv_id);

    /// 设置会话置顶状态
    static bool set_conversation_top(W_SDK_ROOT, const std::string &conv_id, bool is_top);

    /// 设置会话免打扰状态
    static bool set_conversation_mute(W_SDK_ROOT, const std::string &conv_id, bool is_mute);

private:
    /// 获取表名
    static std::string table_name(W_SDK_ROOT);
};

} // namespace roc::imsdk::core::conversation
