#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::core::message {

class ReceiveMessage {
public:
    explicit ReceiveMessage(std::weak_ptr<SDKRoot> sdk_root);

    void Start(CTX_T);

    /// 处理下推消息
    void HandlePushMessage(CTX_T, std::shared_ptr<const network::FrontierMessage> resp);

    /// 处理接收到的消息 (混链、单链拉到的消息， 长链下推的消息)
    boost::asio::awaitable<void> HandleReceiveMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs);

    /// 对消息进行分类
    boost::asio::awaitable<model::OnMessageResult> ClassifyMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs);

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

}