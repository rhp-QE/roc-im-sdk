#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message {

class Convert {
public:
    /// 消息转换 网络消息 -> db 消息
    static std::shared_ptr<core::message::MessageORM> convert_net_msg_to_db_msg(const network::MsgData *msg);

    /// 消息转换 db 消息 -> sdk 消息
    static std::shared_ptr<model::MessageModel> convert_db_msg_to_sdk_msg(W_SDK_ROOT, const core::message::MessageORM *msg);
};

} // namespace roc::imsdk::core::message
