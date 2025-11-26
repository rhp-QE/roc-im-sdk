#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include <memory>

namespace roc::imsdk::core::message {

CmdMessageOperator::CmdMessageOperator(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

void CmdMessageOperator::Start(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto conn_manager= sdk_root->ConnectionManager();

    auto msg_manager = sdk_root->MessageManager();
    conn_manager->AddOnPushMessageCallback([msg_manager, w_sdk_root = w_sdk_root](std::shared_ptr<const network::SdkWSResp> resp) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
        uint32_t call_track_id = resp->trackid();

        msg_manager->cmd_message_operator->p_HandlePushMessage(CTX_V, resp);
    });
}

void CmdMessageOperator::p_HandlePushMessage(CTX_T, std::shared_ptr<const network::SdkWSResp> resp) {
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
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleRecallMessage(CTX_V, cmd_msg), boost::asio::detached);
            break;
        // 删除消息
        case static_cast<int>(common::CmdMessageOp::Delete):
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleDeleteMessage(CTX_V, cmd_msg), boost::asio::detached);
            break;
        // 更新消息
        case static_cast<int>(common::CmdMessageOp::Update):
            boost::asio::co_spawn(sdk_root->net_io_context(), p_HandleUpdateMessage(CTX_V, cmd_msg), boost::asio::detached);
            break;
    }
}

// 撤回消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleRecallMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 删除消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleDeleteMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 更新消息
boost::asio::awaitable<void> CmdMessageOperator::p_HandleUpdateMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();

    std::shared_ptr<network::MsgData> msg_data(cmd_msg->release_msg());

    // 保存消息
    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs = co_await sdk_root->MessageManager()->message_data_source->SaveNetMessages(CTX_V, {msg_data.get()});
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
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageStatusChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    auto msg_manager = sdk_root->MessageManager();

    
}

// 处理消息已读状态发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageReadChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    auto msg_manager = sdk_root->MessageManager();

    
}

// 处理消息置顶状态发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageTopChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 处理消息property发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessagePropertyChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 处理消息syncExt发生变化
boost::asio::awaitable<void> CmdMessageOperator::p_HandleMessageSyncExtChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}


} // namespace roc::imsdk::core::message