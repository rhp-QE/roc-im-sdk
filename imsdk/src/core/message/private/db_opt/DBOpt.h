#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message {

class DBOpt {
public:
    /// 插入消息到数据库
    static bool insert_message(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> messages);
};

} // namespace roc::imsdk::core::message
