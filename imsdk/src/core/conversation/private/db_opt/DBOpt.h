#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

namespace roc::imsdk::core::conversation {

class DBOpt {
public:
    explicit DBOpt(std::weak_ptr<SDKRoot> sdk_root);

    /// 创建会话表（如果不存在）
    bool CreateConversationTableIfNeed(CONTEXT_T);

    /// 插入会话到数据库
    bool InsertConversation(CONTEXT_T, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations);

    /// 捞取会话
    std::vector<std::shared_ptr<model::ConversationModel>> QueryConversations(CONTEXT_T, int64_t cursor, int64_t limit, bool forward = true);

    /// 批量更新会话状态（如已读状态）
    bool UpdateConversationsStatus(CONTEXT_T, const std::vector<std::string> &conv_ids, int status);

    /// 根据会话ID查询会话
    std::shared_ptr<model::ConversationModel> ConversationForId(CONTEXT_T, const std::string &conv_id);

    /// 查询用户的所有会话
    std::vector<std::shared_ptr<core::conversation::ConversationORM>> QueryConversationsForUser(CONTEXT_T, int64_t cursor, int64_t limit);

    /// 删除会话
    bool DeleteConversation(CONTEXT_T, const std::string &conv_id);

    /// 设置会话置顶状态
    bool SetConversationTop(CONTEXT_T, const std::string &conv_id, bool is_top);

    /// 设置会话免打扰状态
    bool SetConversationMute(CONTEXT_T, const std::string &conv_id, bool is_mute);

private:
    /// 获取表名
    std::string p_TableName(CONTEXT_T);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation
