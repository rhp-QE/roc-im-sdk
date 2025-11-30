#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/common/model.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/base/include/containers/ThreadSafeUnorderedMap.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declaration
namespace roc::imsdk::core {
    class MessageManager;
}

namespace roc::imsdk::core::message {

class MessageDataSource {
public:
    explicit MessageDataSource(std::weak_ptr<SDKRoot> sdk_root);

    boost::asio::strand<boost::asio::io_context::executor_type> msg_strand();

    /// 保存网络消息
    boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
        SaveNetMessages(CTX_T, std::vector<const network::MsgData *> msgs);

    /// 保存db消息 (只允许在没有 db 消息的时候调用)
    boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
        SaveDbMsgs(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs);
    
    /// 根据 ID 获取 SDK 消息
    boost::asio::awaitable<std::shared_ptr<model::MessageModel>> 
        SdkMsgForId(CTX_T, const std::string &msg_id);
    
    /// 获取会话的缺失消息区间
    std::vector<std::pair<int64_t, int64_t>> EmptyMessageRangeForConvId(CTX_T, const std::string &conv_id);
    
    /// 设置消息为已读
    bool MarkMessagesAsRead(CTX_T, const std::vector<std::string> &msg_ids);

    /// 从数据库加载消息
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> 
        LoadMessageFromDb(CTX_T, std::string conv_id, int64_t cursor, int64_t limit, bool forward);

    /// 更新消息缓存
    std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> UpdateMsgCache(CTX_T, std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs);

    /// 更新会话的最大 order_index
    void UpdateMsgOrderInConv(CTX_T, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

    /// 更新消息置顶状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessagePinStatus(CTX_T, const std::string &msg_id, bool is_pinned);

    /// 更新消息同步扩展字段（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessageSyncExtStatus(CTX_T, const std::string &msg_id, const std::unordered_map<std::string, std::string> &sync_ext);

    /// 更新消息属性（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessagePropertysStatus(CTX_T, const std::string &msg_id, const std::vector<int32_t> &propertys);

    /// 更新消息本地扩展字段（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessageLocalExtStatus(CTX_T, const std::string &msg_id, const std::unordered_map<std::string, std::string> &local_ext);

    /// 更新消息删除状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessageDeletedStatus(CTX_T, const std::string &msg_id, bool is_deleted);

    /// 更新消息撤回状态（数据库 + 缓存）
    boost::asio::awaitable<bool> 
        UpdateMessageRecalledStatus(CTX_T, const std::string &msg_id, bool is_recalled);

private: 

    /// 保存消息日志
    void p_logSaveMessages(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs);
   
    /// 更新消息区间
    void p_updateMessageRangeForMessage(CTX_T, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

    /// 获取会话的消息区间
    std::vector<std::pair<int64_t, int64_t>> p_messageRangeForConvId(CTX_T, const std::string &conv_id);

    // 给定一个数字序列，生成若干区间。 一个区间内的所有数字都在 给定的数组序列内。 区间内数字是连续的，左右都闭合。
    static std::vector<std::pair<int64_t, int64_t>> p_generateRange(std::vector<int64_t> seqs);

    // 给定两个区间数组，合并两个数组，返回一个新的区间数组。 合并后的区间数组内的区间是连续的，左右都闭合。
    static std::vector<std::pair<int64_t, int64_t>> p_mergeRanges(std::vector<std::pair<int64_t, int64_t>> first, std::vector<std::pair<int64_t, int64_t>> second);

    /// 消息缓存
    base::containers::ThreadSafeUnorderedMap<std::string, std::shared_ptr<model::MessageModel>> message_cache_;

    /// 消息区间缓存
    base::containers::ThreadSafeUnorderedMap<std::string/*conv_id*/, std::vector<std::pair<int64_t, int64_t>>/*msg_ranges*/> message_range_cache_;

    std::weak_ptr<SDKRoot> w_sdk_root;
    
    /// 消息操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> msg_strand_;
};

} // namespace roc::imsdk::core::message
