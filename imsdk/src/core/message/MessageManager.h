#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "base/containers/ThreadSafeUnorderedMap.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"
#include <mutex>

// Forward declaration
namespace roc::imsdk::core::message {
    class SaveMessage;
}

namespace roc::imsdk::core {

class MessageManager : public roc::base::uncopyable {
public:
    MessageManager(std::weak_ptr<SDKRoot> w_sdk_root, boost::asio::io_context::executor_type executor);
    ~MessageManager();

    // 组件加载完成后的初始化
    void all_component_did_load();
    
    // 收到消息回调
    model::OnMessagesCallbackType on_receive_message_callback();

    void handle_receive_message(std::vector<std::shared_ptr<network::MsgData>> net_msgs);

    // 消息操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> msg_strand();

 
    // =============================  message api  ======================================

    /// 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> send_message(model::SendMsgContext contexts, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);

    /// 接收消息回调
    void on_messages(model::OnMessagesCallbackType callback);

    /// 删除消息
    boost::asio::awaitable<bool> delete_message(const std::vector<std::string> &msg_ids);

    /// 撤回消息
    boost::asio::awaitable<bool> recall_message(std::string msg_id);

    /// 修改消息 sync_ext
    boost::asio::awaitable<bool> update_message_sync_ext(std::string msg_id, std::string key, std::string value);

    /// 设置消息为已读
    boost::asio::awaitable<bool> mark_messages_as_read(const std::vector<std::string> &msg_ids);

    /// 查询消息
    boost::asio::awaitable<std::shared_ptr<model::MessageModel>> message_for_id(std::string msg_id);

    /// 查询会话消息
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> messages_for_conv_id(std::string conv_id, int64_t cursor, int64_t limit);

    /// 当进入会话时，获取首屏消息。 后续加载更多消息时使用 messages_for_conv
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> messages_when_enter_chat(std::string conv_id);

    /// ==================================================================================

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;

    /// 消息顺序锁
    std::mutex msg_order_mutex_;

    /// 收到消息回调
    model::OnMessagesCallbackType on_messages_callback_;

    /// 消息操作串行队列
    /// 所有的消息操作 都在这个串行队列中串行执行，确保 db 和 缓存的一致性。
    boost::asio::strand<boost::asio::io_context::executor_type> msg_strand_;

    /// 消息缓存
    base::containers::ThreadSafeUnorderedMap<std::string, std::shared_ptr<model::MessageModel>> msg_cache_;

    /// 消息区间
    base::containers::ThreadSafeUnorderedMap<std::string/*conv_id*/, std::vector<std::pair<int64_t, int64_t>>/*msg_ranges*/> msg_range_cache_;

    // 友元类，允许SaveMessage访问私有成员
    friend class roc::imsdk::core::message::DBOpt;
    friend class roc::imsdk::core::message::Convert;
    friend class roc::imsdk::core::message::SaveMessage;
    friend class roc::imsdk::core::message::ReceiveMessage;
    friend class roc::imsdk::core::message::CmdMessageOperator;


};

} // namespace roc::imsdk::core