#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::core::message {

class ReceiveMessage {
public:
    static void start(W_SDK_ROOT);

    /// 处理下推消息
    static void handle_push_message(W_SDK_ROOT, std::shared_ptr<network::SdkWSResp> resp);

    /// 处理接收到的消息 (混链、单链拉到的消息， 长链下推的消息)
    static void handle_receive_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs);

    /// 对消息进行分类
    static model::OnMessageResult classify_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs);
};

}