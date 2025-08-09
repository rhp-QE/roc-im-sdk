#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

namespace roc::imsdk::core::conversation {

class Convert {
public:
    /// 会话转换 网络会话 -> db 会话
    static std::shared_ptr<core::conversation::ConversationORM> convert_net_conv_to_db_conv(const network::ConversationInfo *conv);

    /// 会话转换 db 会话 -> sdk 会话
    static std::shared_ptr<model::ConversationModel> convert_db_conv_to_sdk_conv(const core::conversation::ConversationORM *conv);
};

} // namespace roc::imsdk::core::conversation
