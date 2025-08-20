#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message {

class DBOpt {
public:
    static bool create_message_table_if_need(W_SDK_ROOT);

    /// 插入消息到数据库
    static bool insert_or_replace_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages);

    static bool insert_or_replace_message_when_net(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages);

    /// 保存区间信息
    static bool save_message_range(W_SDK_ROOT, std::vector<std::pair<int64_t, int64_t>> ranges, std::string conv_id);

    static std::vector<std::pair<int64_t, int64_t>> message_range(W_SDK_ROOT, std::string conv_id);

private:
    static std::string tabel_name(W_SDK_ROOT);
    static std::string message_range_key(W_SDK_ROOT, std::string conv_id);
};

} // namespace roc::imsdk::core::message
