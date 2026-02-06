#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core::message {

class Convert {
public:
    explicit Convert(std::weak_ptr<SDKRoot> sdk_root);

    /// 消息转换 网络消息 -> db 消息
    std::shared_ptr<core::message::MessageORM> ConvertNetMsgToDbMsg(CTX_T, const network::MessageData *msg);

    /// 消息转换 db 消息 -> sdk 消息
    std::shared_ptr<model::MessageModel> ConvertDbMsgToSdkMsgTmp(CTX_T, const core::message::MessageORM *msg);

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::message
