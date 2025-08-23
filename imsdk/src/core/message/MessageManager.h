#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"

// Forward declaration
namespace roc::imsdk::core::message {
    class SaveMessage;
}

namespace roc::imsdk::core {

class MessageManager : public roc::base::uncopyable {
public:
    MessageManager(std::weak_ptr<SDKRoot> w_sdk_root);
    ~MessageManager();

    // 组件加载完成后的初始化
    void all_component_did_load();
    
    // 收到消息回调
    model::OnMessagesCallbackType on_receive_message_callback();

    void handle_receive_message(std::vector<std::shared_ptr<network::MsgData>> net_msgs);

 
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

    /// 收到消息回调
    model::OnMessagesCallbackType on_messages_callback_;

    /// 消息缓存
    std::unordered_map<std::string, std::shared_ptr<model::MessageModel>> msg_cache_;

    /// 消息区间
    std::unordered_map<std::string/*conv_id*/, std::vector<std::pair<int64_t, int64_t>>/*msg_ranges*/> msg_range_cache_;

    // 友元类，允许SaveMessage访问私有成员
    friend class roc::imsdk::core::message::Convert;
    friend class roc::imsdk::core::message::SaveMessage;
    friend class roc::imsdk::core::message::ReceiveMessage;
    friend class roc::imsdk::core::message::CmdMessageOperator;

};

} // namespace roc::imsdk::core