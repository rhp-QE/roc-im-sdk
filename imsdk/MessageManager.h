#pragma once

#include "core/common/macro.h"
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/containers/ThreadSafeUnorderedMap.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <mutex>

// Forward declaration
namespace roc::imsdk::core::message {
    class MessageDataSource;
    class ReceiveMessage;
    class CmdMessageOperator;
    class SendMessageController;
    class DBOpt;
    class Convert;
    class ConvMessagesFetcher;
}

namespace roc::imsdk::core {

class MessageManager : public roc::base::uncopyable {
public:
    MessageManager(std::shared_ptr<SDKRoot> sdk_root, boost::asio::io_context::executor_type executor);
    ~MessageManager();

    // 组件加载完成后的初始化
    void AllComponentDidLoad();
    
    // 收到消息回调
    model::OnMessagesCallbackType OnReceiveMessageCallback();

    void HandleReceiveMessage(CTX_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs);

    // 消息操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> MsgStrand();

 
    // =============================  message api  ======================================

    /// 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> SendMessage(model::SendMsgContext contexts, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);

    /// 接收消息回调
    void OnMessages(model::OnMessagesCallbackType callback);

    /// 删除消息
    boost::asio::awaitable<bool> DeleteMessage(const std::vector<std::string> &msg_ids);

    /// 撤回消息
    boost::asio::awaitable<bool> RecallMessage(std::string msg_id);

    /// 修改消息 sync_ext
    boost::asio::awaitable<bool> UpdateMessageSyncExt(std::string msg_id, std::string key, std::string value);

    /// 设置消息为已读
    boost::asio::awaitable<bool> MarkMessagesAsRead(const std::vector<std::string> &msg_ids);

    /// 查询消息
    boost::asio::awaitable<std::shared_ptr<model::MessageModel>> MessageForId(std::string msg_id);

    /// 查询会话消息
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessagesForConvId(std::string conv_id, int64_t cursor, int64_t limit);

    /// 当进入会话时，获取首屏消息。 后续加载更多消息时使用 messages_for_conv_id
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessagesWhenEnterChat(std::string conv_id);

    /// ==================================================================================

private:
    std::weak_ptr<SDKRoot> w_sdk_root;

    /// 消息顺序锁
    std::mutex msg_order_mutex_;

    /// 收到消息回调
    model::OnMessagesCallbackType on_messages_callback_;

    /// 消息操作串行队列
    /// 所有的消息操作 都在这个串行队列中串行执行，确保 db 和 缓存的一致性。
    boost::asio::strand<boost::asio::io_context::executor_type> msg_strand_;

    // 友元类，允许子组件访问私有成员
    friend class roc::imsdk::core::message::DBOpt;
    friend class roc::imsdk::core::message::Convert;
    friend class roc::imsdk::core::message::MessageDataSource;
    friend class roc::imsdk::core::message::ReceiveMessage;
    friend class roc::imsdk::core::message::CmdMessageOperator;
    friend class roc::imsdk::core::message::SendMessageController;
    friend class roc::imsdk::core::message::ConvMessagesFetcher;

    /// 初始化子组件
    void p_InitSubComponents();

    /// 子组件
    std::unique_ptr<message::DBOpt> db_opt;
    std::unique_ptr<message::Convert> convert;
    std::unique_ptr<message::MessageDataSource> message_data_source;
    std::unique_ptr<message::ReceiveMessage> receive_message;
    std::unique_ptr<message::CmdMessageOperator> cmd_message_operator;
    std::unique_ptr<message::ConvMessagesFetcher> conv_messages_fetcher;
    std::unique_ptr<message::SendMessageController> send_message_controller;
};

} // namespace roc::imsdk::core