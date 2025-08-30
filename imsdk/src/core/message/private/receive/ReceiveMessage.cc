#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include <memory>

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

    // 处理下推的用户消息
    if (resp->type() != static_cast<int>(common::SdkWsEnum::PUSH_USER_MESSAGE)) {
        return;
    }
    
    if (resp->data().empty()) {
        return;
    }

    std::shared_ptr<network::MsgData> net_msg = std::make_shared<network::MsgData>();
    if (!net_msg->ParseFromString(resp->data())) {
        return;
    }

    boost::asio::co_spawn(sdk_root->net_io_context(), handle_receive_message(w_sdk_root, {net_msg}), boost::asio::detached);

    std::cout<<"receive push message"<<std::endl;
}

boost::asio::awaitable<void> ReceiveMessage::handle_receive_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs) {
    if (net_msgs.empty()) {
        co_return;
    }

    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();

    std::vector<const network::MsgData *> net_msgs_ptr;
    for (const auto &msg : net_msgs) {
        net_msgs_ptr.push_back(msg.get());
    }

    /// 数据保存
    auto sdk_msgs = co_await message::SaveMessage::save_net_messages(w_sdk_root, net_msgs_ptr);
    
    // 对消息进行分类
    auto result = classify_message(w_sdk_root, net_msgs, sdk_msgs);
    
    // 上抛消息
    base::util::safe_invoke_block(msg_manager->on_messages_callback_, result);
}

model::OnMessageResult ReceiveMessage::classify_message(W_SDK_ROOT, std::vector<std::shared_ptr<network::MsgData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, model::OnMessageResult())

    model::OnMessageResult result;

    std::unordered_map<std::string, std::vector<std::shared_ptr<model::MessageModel>>> sdk_msg_map = base::util::group_by_key(sdk_msgs, [](const std::shared_ptr<model::MessageModel> &msg) {
        return msg->server_msg_id();
    });

    for (const auto &msg : net_msgs) {
        /// 实时消息
        if (msg->dstatus() == static_cast<int32_t>(common::MsgDStatus::RealTime)) {
            result.real_time_msgs.push_back(sdk_msg_map[msg->servermsgid()].front());
            continue;
        }

        bool is_received = msg->dstatus() == static_cast<int32_t>(common::MsgDStatus::Received);

        /// 离线消息中 被接收过的消息
        if (is_received) {
            result.offline_received_msgs.push_back(sdk_msg_map[msg->servermsgid()].front());
            continue;
        }

        /// 离线消息中 没有被接收过的消息
        if (!is_received) {
            result.offline_not_received_msgs.push_back(sdk_msg_map[msg->servermsgid()].front());
            continue;
        }
    }
    
    return result;
}

}
