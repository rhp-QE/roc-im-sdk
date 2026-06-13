#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::core::message {

class ReceiveMessage {
public:
    explicit ReceiveMessage(std::weak_ptr<SDKRoot> sdk_root);

    void Start(CTX_T);

    /// 处理接收到的消息。online_push 来自 WebSocket PUSH_USER_MESSAGE 入口，比 MessageData.dstatus 更能表达本次传输语义。
    boost::asio::awaitable<void> HandleMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs, bool online_push = false);

    /// 处理离线消息
    boost::asio::awaitable<void> HandleOfflineMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs);

    /// 对消息进行分类
    boost::asio::awaitable<model::OnMessageResult> ClassifyMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs, bool online_push = false);

private:

    /// 处理在线消息
    void p_HandleOnlineMessage(CTX_T, std::shared_ptr<const network::FrontierMessage> resp);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

}
