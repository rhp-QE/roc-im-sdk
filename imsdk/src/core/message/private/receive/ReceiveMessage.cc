#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"

static int PUSH_USER_MESSAGE_TYPE = 4001;

namespace roc::imsdk::core::message {

void ReceiveMessage::start(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto conn = sdk_root->connection_manager();
    CHECK_POINTER_OR_RETURN_VOID(conn);

    conn->set_on_push_message_callback([w_sdk_root](std::shared_ptr<network::SdkWSResp> resp) {
        message::ReceiveMessage::handle_push_message(w_sdk_root, resp);
    });
}

void ReceiveMessage::handle_push_message(W_SDK_ROOT, std::shared_ptr<network::SdkWSResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    if (resp->type() != PUSH_USER_MESSAGE_TYPE) {
        return;
    }

    std::cout<<"receive push message"<<std::endl;

    /// 数据解析
    int a = 100;
}

void ReceiveMessage::handle_receive_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    std::vector<const network::MsgData *> net_msgs_ptr;
    for (const auto &msg : net_msgs) {
        net_msgs_ptr.push_back(msg.get());
    }

    message::SaveMessage::save_net_msgs(w_sdk_root, net_msgs_ptr);

}

}
