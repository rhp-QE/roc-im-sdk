#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "model/conversation/ConversationModel.h"
#include "model/message/MessageModel.h"
#include <boost/asio/awaitable.hpp>
#include <cstdint>
#include <memory>

static int PUSH_USER_MESSAGE_TYPE = 4001;

namespace roc::imsdk::core::message {

ReceiveMessage::ReceiveMessage(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

void ReceiveMessage::Start(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto conn = sdk_root->ConnectionManager();

    auto msg_manager = sdk_root->MessageManager();
    conn->AddOnPushMessageCallback([msg_manager](std::shared_ptr<const network::SdkWSResp> resp) {
        uint32_t call_track_id = resp->trackid();
        msg_manager->receive_message->HandlePushMessage(call_track_id, resp);
    });
}

void ReceiveMessage::HandlePushMessage(CTX_T, std::shared_ptr<const network::SdkWSResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    // 处理下推的用户消息
    if (resp->method() != static_cast<int32_t>(common::SDKWSMethod::PUSH_USER_MESSAGE)) {
        return;
    }
    
    if (resp->data().empty()) {
        return;
    }

    std::shared_ptr<network::MsgData> net_msg = std::make_shared<network::MsgData>();
    if (!net_msg->ParseFromString(resp->data())) {
        return;
    }

    LOG_INFO("MsgManager", "receive_message, from: {}", net_msg->sendid());

    boost::asio::co_spawn(sdk_root->net_io_context(), HandleReceiveMessage(CTX_V, {net_msg}), boost::asio::detached);
}

boost::asio::awaitable<void> ReceiveMessage::HandleReceiveMessage(CTX_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs) {
    if (net_msgs.empty()) {
        co_return;
    }

    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();

    std::vector<const network::MsgData *> net_msgs_ptr;
    for (const auto &msg : net_msgs) {
        net_msgs_ptr.push_back(msg.get());
    }

    /// 数据保存
    auto sdk_msgs = co_await msg_manager->message_data_source->SaveNetMessages(CTX_V, net_msgs_ptr);
    
    LOG_INFO("MsgManager", "handle_receive_message, size: {}", sdk_msgs.size());

    // 对消息进行分类
    auto result = co_await ClassifyMessage(CTX_V, net_msgs, sdk_msgs);
    
    // 上抛消息
    base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
}

boost::asio::awaitable<model::OnMessageResult> ReceiveMessage::ClassifyMessage(CTX_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, model::OnMessageResult())

    model::OnMessageResult result;
    auto conv_manager = sdk_root->ConversationManager();

    std::unordered_map<std::string, std::vector<std::shared_ptr<model::MessageModel>>> sdk_msg_map = base::util::group_by_key(sdk_msgs, [](const std::shared_ptr<model::MessageModel> &msg) {
        return msg->server_msg_id();
    });

    for (const auto &msg : net_msgs) {
        std::shared_ptr<model::MessageModel> sdk_msg = sdk_msg_map[msg->servermsgid()].front();
        // std::shared_ptr<model::ConversationModel> sdk_conv = co_await conv_manager->conv_for_id(sdk_msg->conversation_id());

        // if (!sdk_conv || !sdk_msg) {
        //     continue;
        // }

        // /// 会话信息
        // result.convs[sdk_conv->conversation_id()] = sdk_conv;

        /// 实时消息
        if (msg->dstatus() == static_cast<int32_t>(common::MsgDStatus::RealTime)) {
            result.real_time_msgs.push_back(sdk_msg);
            continue;
        }

        bool is_received = msg->dstatus() == static_cast<int32_t>(common::MsgDStatus::Received);

        /// 离线消息中 被接收过的消息
        if (is_received) {
            result.offline_received_msgs.push_back(sdk_msg);
            continue;
        }

        /// 离线消息中 没有被接收过的消息
        if (!is_received) {
            result.offline_not_received_msgs.push_back(sdk_msg);
            continue;
        }
    }
    
    co_return std::move(result);
}

}
