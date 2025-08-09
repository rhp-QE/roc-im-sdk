#pragma once

#include "imsdk/src/include/IMSDK.h"

#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"

namespace roc::imsdk::core {

class MessageManager : public roc::base::uncopyable {
public:
    MessageManager();
    ~MessageManager();
    
    // 消息缓存
    void set_sdk_msg(const core::message::MessageORM *msg);
    std::shared_ptr<model::MessageModel> sdk_msg_for_id(std::string msg_id);

    // 收到消息回调
    void set_on_recv_message_callback(model::OnReceiveMessagesCallbackType callback);
    model::OnReceiveMessagesCallbackType on_recv_message_callback();

    // 消息区间
    void update_messgae_range_for_message(std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs);
    std::vector<std::pair<int64_t, int64_t>> message_range_for_conv_id(std::string conv_id);

    // 保存网络消息
    std::vector<std::shared_ptr<model::MessageModel>> save_net_msgs(std::vector<const network::MsgData *> msgs);

    // 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> send_message(std::vector<model::SendMsgContext> contexts);

    // 生成 uuid
    std::string generate_client_msg_id();

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;

    /// 收到消息回调
    model::OnReceiveMessagesCallbackType on_recv_message_callback_;

    /// 消息缓存
    std::unordered_map<std::string, std::shared_ptr<model::MessageModel>> msg_cache_;

    /// convert ---------------------
    std::shared_ptr<core::message::MessageORM> convert_net_msg_to_db_msg(const network::MsgData *msg);
    std::shared_ptr<model::MessageModel> convert_db_msg_to_sdk_msg(const core::message::MessageORM *msg);
    // -----------------------------
};

} // namespace roc::imsdk::core