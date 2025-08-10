#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/MessageManager.h"

namespace roc::imsdk::core::message {

class ReceiveMessage {
public:
    static void start(W_SDK_ROOT);
    static void handle_push_message(W_SDK_ROOT, std::shared_ptr<network::SdkWSResp> resp);
    static void handle_receive_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs);
};

}