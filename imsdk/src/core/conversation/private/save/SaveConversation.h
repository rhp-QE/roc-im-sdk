#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"

#include <memory>
#include <unordered_map>
#include <vector>

// Forward declaration
namespace roc::imsdk::core {
    class ConversationManager;
}

namespace roc::imsdk::core::conversation {

class SaveConversation {
public:
    /// 保存网络会话
    static std::vector<std::shared_ptr<model::ConversationModel>> save_net_convs(W_SDK_ROOT, std::vector<const network::ConversationInfo *> convs);
    
    /// 设置 SDK 会话
    static void set_sdk_conv(W_SDK_ROOT, const core::conversation::ConversationORM *conv);
    
    /// 根据 ID 获取 SDK 会话
    static std::shared_ptr<model::ConversationModel> sdk_conv_for_id(W_SDK_ROOT, const std::string &conv_id);
    
    /// 获取会话游标
    static int64_t get_cursor(W_SDK_ROOT);
    
    /// 设置会话游标
    static void set_cursor(W_SDK_ROOT, int64_t cursor);

private:
    /// 更新会话缓存
    static void update_conv_cache(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> &sdk_convs);
};

} // namespace roc::imsdk::core::conversation
