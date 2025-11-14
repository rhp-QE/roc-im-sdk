#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::core::message {

class ReceiveMessage {
public:
    static void start(CONTEXT_T);

    /// 处理下推消息
    static void handle_push_message(CONTEXT_T, std::shared_ptr<network::SdkWSResp> resp);

    /// 处理接收到的消息 (混链、单链拉到的消息， 长链下推的消息)
    static boost::asio::awaitable<void> handle_receive_message(CONTEXT_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs);

    /// 对消息进行分类
    static boost::asio::awaitable<model::OnMessageResult> classify_message(CONTEXT_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs);
};

}