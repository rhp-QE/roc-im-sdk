#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message {

class DBOpt {
public:
    static bool create_message_table_if_need(W_SDK_ROOT);

    /// 插入消息到数据库
    static bool insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages);

private:
    static std::string tabel_name(W_SDK_ROOT);
};

} // namespace roc::imsdk::core::message
