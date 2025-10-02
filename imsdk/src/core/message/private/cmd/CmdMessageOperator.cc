#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include <memory>

namespace roc::imsdk::core::message {

void CmdMessageOperator::start(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    auto conn = sdk_root->connection_manager();

    conn->add_on_push_message_callback([w_sdk_root](std::shared_ptr<network::SdkWSResp> resp) {
        uint32_t call_track_id = resp->trackid();
        message::CmdMessageOperator::handle_push_message(CONTEXT_V, resp);
    });
}

void CmdMessageOperator::handle_push_message(CONTEXT_T, std::shared_ptr<network::SdkWSResp> resp) {
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
            boost::asio::co_spawn(sdk_root->net_io_context(), handle_recall_message(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
        // 删除消息
        case static_cast<int>(common::CmdMessageOp::Delete):
            boost::asio::co_spawn(sdk_root->net_io_context(), handle_delete_message(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
        // 更新消息
        case static_cast<int>(common::CmdMessageOp::Update):
            boost::asio::co_spawn(sdk_root->net_io_context(), handle_update_message(CONTEXT_V, cmd_msg), boost::asio::detached);
            break;
    }
}

// 撤回消息
boost::asio::awaitable<void> CmdMessageOperator::handle_recall_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 删除消息
boost::asio::awaitable<void> CmdMessageOperator::handle_delete_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);
}

// 更新消息
boost::asio::awaitable<void> CmdMessageOperator::handle_update_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_CO_RETURN_VOID(msg_manager);

    std::shared_ptr<network::MsgData> msg_data(cmd_msg->release_msg());

    // 保存消息
    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs = co_await message::SaveMessage::save_net_messages(CONTEXT_V, {msg_data.get()});
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

} // namespace roc::imsdk::core::message