#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/base/include/containers/ThreadSafeUnorderedMap.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declaration
namespace roc::imsdk::core {
    class ConversationManager;
}

namespace roc::imsdk::core::conversation {

class ConvDatasource {
public:
    explicit ConvDatasource(std::weak_ptr<SDKRoot> sdk_root);

    boost::asio::strand<boost::asio::io_context::executor_type> ConvStrand();

    /// 保存网络会话
    boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        SaveNetConversations(CTX_T, std::vector<std::shared_ptr<network::ConversationInfo>> convs);
    
    /// 根据 ID 获取 SDK 会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        SdkConvForId(CTX_T, const std::string &conv_id);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
        LoadConvsFromDb(CTX_T, int64_t cursor, int64_t limit, bool forward);

    /// 根据 ID 获取 SDK 会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        SdkConvForIdFromCache(CTX_T, const std::string &conv_id);

    /// 更新会话置顶状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationTopStatus(CTX_T, const std::string &conv_id, bool is_top);

    /// 更新会话免打扰状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationMuteStatus(CTX_T, const std::string &conv_id, bool is_mute);

    /// 更新会话拉黑状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationBlockStatus(CTX_T, const std::string &conv_id, bool is_block);

    /// 更新会话同步扩展字段（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationSyncExtStatus(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &sync_ext);

    /// 更新会话本地扩展字段（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationLocalExtStatus(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &local_ext);

    /// 更新会话删除状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateConversationDeletedStatus(CTX_T, const std::string &conv_id, bool is_deleted);
    
private:
    /// 更新会话缓存 (非线程安全， )
    boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
        p_UpdateConvCache(CTX_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs);

    std::weak_ptr<SDKRoot> w_sdk_root;
    
    /// 会话缓存
    base::containers::ThreadSafeUnorderedMap<std::string, std::shared_ptr<model::ConversationModel>> conv_cache_;
    
    /// 会话操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> conv_strand_;
};

} // namespace roc::imsdk::core::conversation

