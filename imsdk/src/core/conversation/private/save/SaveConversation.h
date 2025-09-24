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
    /// 保存网络会话
    static boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        save_net_conversations(CONTEXT_T, std::vector<std::shared_ptr<network::ConversationInfo>> convs);
    
    /// 根据 ID 获取 SDK 会话
    static boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        sdk_conv_for_id(CONTEXT_T, const std::string &conv_id);

    /// 查询会话
    static boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
        load_convs_from_db(CONTEXT_T, int64_t cursor, int64_t limit, bool forward);
    
private:
    /// 更新会话缓存 (非线程安全， )
    static boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        update_conv_cache(CONTEXT_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs);
};

} // namespace roc::imsdk::core::conversation
