#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"

#include <boost/asio/awaitable.hpp>
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
    explicit SaveConversation(std::weak_ptr<SDKRoot> sdk_root);

    /// 保存网络会话
    boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        SaveNetConversations(CTX_T, std::vector<std::shared_ptr<network::ConversationInfo>> convs);
    
    /// 根据 ID 获取 SDK 会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        SdkConvForId(CTX_T, const std::string &conv_id);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
        LoadConvsFromDb(CTX_T, int64_t cursor, int64_t limit, bool forward);
    
private:
    /// 更新会话缓存 (非线程安全， )
    boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        p_UpdateConvCache(CTX_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation
