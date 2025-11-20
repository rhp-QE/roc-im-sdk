#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include <memory>

namespace roc::imsdk::core::message {

void CmdMessageOperator::Start(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto conn_manager= sdk_root->ConnectionManager();

    conn_manager->AddOnPushMessageCallback([w_sdk_root](std::shared_ptr<network::SdkWSResp> resp) {
        uint32_t call_track_id = resp->trackid();
        message::CmdMessageOperator::p_HandlePushMessage(CONTEXT_V, resp);
    });
}

void CmdMessageOperator::p_HandlePushMessage(CONTEXT_T, std::shared_ptr<network::SdkWSResp> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    // 必须是命令消息
    if (resp->type() != static_cast<int>(common::SdkWsEnum::PUSH_CMD_MESSAGE)) {
        return;
    }

    std::shared_ptr<network::CmdMessage> cmd_msg = std::make_shared<network::CmdMessage>();
    if (!cmd_msg->ParseFromString(resp->data())) {
        return;
    }

    switch (cmd_msg->cmd()) {
        // 撤回消息
        case static_cast<int>(common::CmdMessageOp::Recall):
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleRecallMessage(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
        // 删除消息
        case static_cast<int>(common::CmdMessageOp::Delete):
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleDeleteMessage(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
        // 更新消息
        case static_cast<int>(common::CmdMessageOp::Update):
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleUpdateMessage(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
    }
}

// 撤回消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleRecallMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 删除消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleDeleteMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 更新消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleUpdateMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();

    std::shared_ptr<network::MsgData> msg_data(cmd_msg->release_msg());

    // 保存消息
    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs = co_await message::SaveMessage::SaveNetMessages(CONTEXT_V, {msg_data.get()});
    if (sdk_msgs.empty()) {
        co_return;
    }

    model::OnMessageResult result{
        .deleted_msgs = {},
        .recalled_msgs = {},
        .updated_msgs = {sdk_msgs[0]},
        .real_time_msgs = {},
        .offline_not_received_msgs = {},
        .offline_received_msgs = {},
        .convs = {},
    };

    // 更新上抛
    base::util::safe_invoke_block(msg_manager->on_messages_callback_, result);
}

// 处理消息状态发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageStatusChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    auto msg_manager = sdk_root->MessageManager();

    
}

// 处理消息已读状态发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageReadChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    auto msg_manager = sdk_root->MessageManager();

    
}

// 处理消息置顶状态发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageTopChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 处理消息property发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessagePropertyChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 处理消息syncExt发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageSyncExtChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}


} // namespace roc::imsdk::core::message