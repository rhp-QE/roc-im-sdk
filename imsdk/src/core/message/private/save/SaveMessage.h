#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/common/model.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <memory>
#include <unordered_map>
#include <vector>

// Forward declaration
namespace roc::imsdk::core {
    class MessageManager;
}

namespace roc::imsdk::core::message {

class SaveMessage {
public:
    /// 保存网络消息
    static boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
        save_net_msgs(W_SDK_ROOT, std::vector<const network::MsgData *> msgs);

    /// 保存db消息 (只允许在没有 db 消息的时候调用)
    static boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
        save_db_msgs(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs);
    
    /// 根据 ID 获取 SDK 消息
    static boost::asio::awaitable<std::shared_ptr<model::MessageModel>> 
        sdk_msg_for_id(W_SDK_ROOT, const std::string &msg_id);
    
    /// 获取会话的缺失消息区间
    static std::vector<std::pair<int64_t, int64_t>> empty_message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id);
    
    /// 生成客户端消息 ID
    static std::string generate_client_msg_id();
    
    /// 设置消息为已读
    static bool mark_messages_as_read(W_SDK_ROOT, const std::vector<std::string> &msg_ids);

    /// 从数据库加载消息
    static boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> 
        load_message_from_db(W_SDK_ROOT, std::string conv_id, int64_t cursor, int64_t limit, bool forward);

    /// 从数据库中加载缓存区间
    static std::vector<std::pair<int64_t, int64_t>> load_message_range_from_db(W_SDK_ROOT, const std::string &conv_id);

    /// 更新消息缓存
    static std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> update_msg_cache(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

    /// 更新会话的最大 order_index
    static void update_msg_order_in_conv(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

private:
   
    /// 更新消息区间
    static void update_message_range_for_message(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

    /// 获取会话的消息区间
    static std::vector<std::pair<int64_t, int64_t>> message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id);

    // 给定一个数字序列，生成若干区间。 一个区间内的所有数字都在 给定的数组序列内。 区间内数字是连续的，左右都闭合。
    static std::vector<std::pair<int64_t, int64_t>> generate_range(std::vector<int64_t> seqs);

    // 给定两个区间数组，合并两个数组，返回一个新的区间数组。 合并后的区间数组内的区间是连续的，左右都闭合。
    static std::vector<std::pair<int64_t, int64_t>> merge_ranges(std::vector<std::pair<int64_t, int64_t>> first, std::vector<std::pair<int64_t, int64_t>> second);
};

} // namespace roc::imsdk::core::message
