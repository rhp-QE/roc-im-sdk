#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include <cstdint>

namespace roc::imsdk::core::message {

class DBOpt {
public:
    static bool create_message_table_if_need(CONTEXT_T);

    /// 插入消息到数据库
    static bool insert_or_replace_message(CONTEXT_T, std::vector<std::shared_ptr<core::message::MessageORM>> messages);


    /// 插入消息到数据库
    static bool insert_or_update_message(
        CONTEXT_T, 
        std::vector<std::shared_ptr<core::message::MessageORM>> messages, /*消息列表*/
        const WCDB::Fields& fields, /*需要更改的成员*/
        int type = 0 /*0 白名单， 黑名单*/
    );


    /// 保存区间信息
    static bool save_message_range(CONTEXT_T, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id);


    /// 获取消息区间
    static std::vector<std::pair<int64_t, int64_t>> message_range(CONTEXT_T, std::string conv_id);


    /// 获取消息 (直接从DB 中取)
    static std::shared_ptr<model::MessageModel> message_for_id(CONTEXT_T, std::string msg_id);


    /// 获取会话消息
    static std::vector<std::shared_ptr<model::MessageModel>> query_messages_for_conv_id(
        CONTEXT_T,
        std::string conv_id,
        int64_t cursor,
        int64_t limit,
        bool forward = true
    );


    /// 查询消息并设置优选使用的本地字段
    static void message_merge_with_local(CONTEXT_T, std::string msg_id, message::MessageORM *db_msg_new);


    /// 设置会话最大 order_index
    static void set_msg_order_in_conv(CONTEXT_T, std::string conv_id, int64_t order);


    /// 获取会话最大 order_index
    static int64_t next_msg_order_in_conv(CONTEXT_T, std::string conv_id);

private:
    static std::string tabel_name(CONTEXT_T);
    static std::string message_range_key(CONTEXT_T, std::string conv_id);
    static std::string order_index_key(CONTEXT_T, std::string conv_id);
};

} // namespace roc::imsdk::core::message
