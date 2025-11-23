#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include <cstdint>

namespace roc::imsdk::core::message {

class DBOpt {
public:
    explicit DBOpt(std::weak_ptr<SDKRoot> sdk_root);

    bool CreateMessageTableIfNeed(CTX_T);

    /// 插入消息到数据库
    bool InsertOrReplaceMessage(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> messages);


    /// 插入消息到数据库
    bool InsertOrUpdateMessage(
        CTX_T, 
        std::vector<std::shared_ptr<core::message::MessageORM>> messages, /*消息列表*/
        const WCDB::Fields& fields, /*需要更改的成员*/
        int type = 0 /*0 白名单， 黑名单*/
    );


    /// 保存区间信息
    bool SaveMessageRange(CTX_T, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id);


    /// 获取消息区间
    std::vector<std::pair<int64_t, int64_t>> MessageRange(CTX_T, std::string conv_id);


    /// 获取消息 (直接从DB 中取)
    std::shared_ptr<model::MessageModel> MessageForId(CTX_T, std::string msg_id);


    /// 获取会话消息
    std::vector<std::shared_ptr<model::MessageModel>> QueryMessagesForConvId(
        CTX_T,
        std::string conv_id,
        int64_t cursor,
        int64_t limit,
        bool forward = true
    );


    /// 查询消息并设置优选使用的本地字段
    void MessageMergeWithLocal(CTX_T, std::string msg_id, message::MessageORM *db_msg_new);


    /// 设置会话最大 order_index
    void SetMsgOrderInConv(CTX_T, std::string conv_id, int64_t order);


    /// 获取会话最大 order_index
    int64_t NextMsgOrderInConv(CTX_T, std::string conv_id);

private:
    std::string p_TableName(CTX_T);
    std::string p_MessageRangeKey(CTX_T, std::string conv_id);
    std::string p_OrderIndexKey(CTX_T, std::string conv_id);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::message
