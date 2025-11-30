#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include <unordered_map>
#include <string>

namespace roc::imsdk::core::conversation {

class DBOpt {
public:
    explicit DBOpt(std::weak_ptr<SDKRoot> sdk_root);

    /// 创建会话表（如果不存在）
    bool CreateConversationTableIfNeed(CTX_T);

    /// 插入会话到数据库
    bool InsertConversation(CTX_T, std::vector<std::shared_ptr<core::conversation::ConversationORM>> conversations);

    /// 捞取会话
    std::vector<std::shared_ptr<model::ConversationModel>> QueryConversations(CTX_T, int64_t cursor, int64_t limit, bool forward = true);

    /// 批量更新会话状态（如已读状态）
    bool UpdateConversationsStatus(CTX_T, const std::vector<std::string> &conv_ids, int status);

    /// 根据会话ID查询会话
    std::shared_ptr<model::ConversationModel> ConversationForId(CTX_T, const std::string &conv_id);

    /// 查询用户的所有会话
    std::vector<std::shared_ptr<core::conversation::ConversationORM>> QueryConversationsForUser(CTX_T, int64_t cursor, int64_t limit);

    /// 删除会话
    bool DeleteConversation(CTX_T, const std::string &conv_id);

    /// 设置会话置顶状态
    bool SetConversationTop(CTX_T, const std::string &conv_id, bool is_top);

    /// 设置会话免打扰状态
    bool SetConversationMute(CTX_T, const std::string &conv_id, bool is_mute);
    
    /// 设置会话拉黑状态
    bool SetConversationBlock(CTX_T, const std::string &conv_id, bool is_block);

    /// 设置会话同步扩展字段（会与现有字段合并）
    bool SetConversationSyncExt(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &sync_ext);

    /// 设置会话本地扩展字段（会与现有字段合并）
    bool SetConversationLocalExt(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &local_ext);

    /// 设置会话水位
    void SetChatsCursor(CTX_T, int64_t cursor);

    /// 获取会话水位
    int64_t ChatsCursor(CTX_T);

    /// 查询会话并合并本地独有字段
    void ConversationMergeWithLocal(CTX_T, const std::string &conv_id, core::conversation::ConversationORM *db_conv_new);

private:
    /// 获取表名
    std::string p_TableName(CTX_T);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation
