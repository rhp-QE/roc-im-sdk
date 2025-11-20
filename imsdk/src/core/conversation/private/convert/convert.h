#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

namespace roc::imsdk::core::conversation {

class Convert {
public:
    /// 会话转换 网络会话 -> db 会话
    static std::shared_ptr<core::conversation::ConversationORM> ConvertNetConvToDbConv(const network::ConversationInfo *conv);

    /// 会话转换 db 会话 -> sdk 会话
    static std::shared_ptr<model::ConversationModel> ConvertDbConvToSdkConv(CONTEXT_T, const core::conversation::ConversationORM *conv);
};

} // namespace roc::imsdk::core::conversation
