#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/src/core/network/connection/FrontierMessageUtility.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/include/model/network.h"
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

    conn->AddOnPushMessageCallback([w_sdk_root = w_sdk_root](std::shared_ptr<const network::FrontierMessage> resp) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
        uint32_t call_track_id = network::FrontierMessageUtility::ExtractTrackId(*resp).value_or(0);
        
        auto msg_manager = sdk_root->MessageManager();
        msg_manager->receive_message->p_HandleOnlineMessage(call_track_id, resp);
    });
}

void ReceiveMessage::p_HandleOnlineMessage(CTX_T, std::shared_ptr<const network::FrontierMessage> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    // 处理下推的用户消息
    if (resp->method != std::to_string(static_cast<int32_t>(common::SDKWSMethod::PUSH_USER_MESSAGE))) {
        return;
    }
    
    if (resp->payload.empty()) {
        return;
    }

    auto net_msg = std::make_shared<network::MessageData>();
    if (!net_msg->ParseFromArray(resp->payload.data(), static_cast<int>(resp->payload.size()))) {
        return;
    }

    LOG_INFO("MsgManager", "receive_message, from: {}", net_msg->sendid());

    boost::asio::co_spawn(sdk_root->net_io_context(), HandleMessage(CTX_V, {net_msg}, true), boost::asio::detached);
}

boost::asio::awaitable<void> ReceiveMessage::HandleOfflineMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    /// 离线拉取到的消息仍按 dstatus 做补偿分类。
    co_return co_await HandleMessage(CTX_V, std::move(net_msgs), false);
}

boost::asio::awaitable<void> ReceiveMessage::HandleMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs, bool online_push) {
    if (net_msgs.empty()) {
        co_return;
    }

    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();

    std::vector<const network::MessageData *> net_msgs_ptr;
    for (const auto &msg : net_msgs) {
        net_msgs_ptr.push_back(msg.get());
    }

    /// 数据保存
    auto sdk_msgs = co_await msg_manager->message_data_source->SaveNetMessages(CTX_V, net_msgs_ptr);
    
    LOG_INFO("MsgManager", "handle_receive_message, size: {}", sdk_msgs.size());
    if (sdk_msgs.size() != net_msgs.size()) {
        LOG_INFO("MsgManager", "save net message maby error, net_msg_size: {}, sdk_msg_size:{}", net_msgs.size(), sdk_msgs.size());
    }

    // 对消息进行分类
    auto result = co_await ClassifyMessage(CTX_V, net_msgs, sdk_msgs, online_push);
    
    // 上抛消息
    base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
}

boost::asio::awaitable<model::OnMessageResult> ReceiveMessage::ClassifyMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs, std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs, bool online_push) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, model::OnMessageResult())

    model::OnMessageResult result;
    auto conv_manager = sdk_root->ConversationManager();

    std::unordered_map<std::string, std::vector<std::shared_ptr<model::MessageModel>>> sdk_msg_map = base::util::group_by_key(sdk_msgs, [](const std::shared_ptr<model::MessageModel> &msg) {
        return msg->server_msg_id();
    });

    for (const auto &msg : net_msgs) {
        auto it = sdk_msg_map.find(msg->smessageid());
        if (it == sdk_msg_map.end()) {
            LOG_ERROR("ReceiveMessage", "ClassifyMessgae Error canot find server_message_id = {}", msg->smessageid());
        }
        std::shared_ptr<model::MessageModel> sdk_msg = (*it).second.front();
        // std::shared_ptr<model::ConversationModel> sdk_conv = co_await conv_manager->conv_for_id(sdk_msg->conversation_id());

        // if (!sdk_conv || !sdk_msg) {
        //     continue;
        // }

        // /// 会话信息
        // result.convs[sdk_conv->conversation_id()] = sdk_conv;

        // WebSocket PUSH_USER_MESSAGE 已经表达了在线推送语义；dstatus 只用于离线拉取后的补偿分类。
        if (online_push) {
            result.real_time_msgs.push_back(sdk_msg);
            continue;
        }

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
