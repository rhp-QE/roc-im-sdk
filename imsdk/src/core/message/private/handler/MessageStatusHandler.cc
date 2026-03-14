#include "imsdk/src/core/message/private/handler/MessageStatusHandler.h"
#include "core/common/macro.h"
#include "core/common/logger_macro.h"
#include "core/common/sdkwsEnum.h"
#include "core/network/proto/sdkws.pb.h"
#include "core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_composed.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <cstdint>
#include <memory>
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/common/json_util.h"
#include "imsdk/src/include/model/network.h"

#include "imsdk/src/core/cmd/CmdCenter.h"

namespace roc::imsdk::core::message {

MessageStatusHandler::MessageStatusHandler(std::weak_ptr<SDKRoot> root) :
    w_sdk_root(root)
{}

void MessageStatusHandler::AllComponentDidLoad(CTX_T) {
    p_registPinHandler(CTX_V);
    p_registSyncExtHandler(CTX_V);
    p_registPropertyHandler(CTX_V);
    p_registDeleteHandler(CTX_V);
    p_registRecallHandler(CTX_V);
}

/// 设置消息置顶状态
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetPin(CTX_T, std::string msg_id, bool is_pinned) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    co_return true;
}

/// 设置消息同步扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetSyncExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // // 序列化 map 到 JSON string
    // auto sync_ext_str_result = json_util::MapSerializeAsString(sync_ext);
    // if (!sync_ext_str_result) {
    //     co_return std::unexpected(sync_ext_str_result.error());
    // }
    // std::string sync_ext_str = sync_ext_str_result.value();
    
    // auto cmd_msg = std::make_unique<network::CmdMessage>();
    // cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_SYNC_EXT_CHANGED));
    // auto* msg_data = cmd_msg->mutable_message();
    // msg_data->set_cmessaegid(msg_id);
    // msg_data->set_syncext(sync_ext_str);
    
    // auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    // if (!resp) {
    //     co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    // }
    
    // // 检查响应错误码
    // if (resp->errorcode() != 0) {
    //     co_return std::unexpected(roc::error::make_error(
    //         static_cast<int>(resp->errorcode()),
    //         resp->error().empty() ? "Set sync_ext failed" : resp->error()
    //     ));
    // }
    
    // // 成功请求后更新本地数据库和缓存
    // auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    // bool update_result = co_await msg_ds->UpdateMessageSyncExtStatus(CTX_V, msg_id, sync_ext);
    // if (!update_result) {
    //     co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    // }
    
    co_return true;
}

/// 设置消息属性
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetPropertys(CTX_T, std::string msg_id, const std::vector<int32_t> &propertys) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // // 序列化 vector 到 JSON string
    // auto propertys_str_result = json_util::Int32VectorSerializeAsString(propertys);
    // if (!propertys_str_result) {
    //     co_return std::unexpected(propertys_str_result.error());
    // }
    // std::string propertys_str = propertys_str_result.value();
    
    // auto cmd_msg = std::make_unique<network::CmdMessage>();
    // cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_PROPERTY_CHANGED));
    // auto* msg_data = cmd_msg->mutable_message();
    // msg_data->set_cmessaegid(msg_id);
    // msg_data->set_propertys(propertys_str);
    
    // auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    // if (!resp) {
    //     co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    // }
    
    // // 检查响应错误码
    // if (resp->errorcode() != 0) {
    //     co_return std::unexpected(roc::error::make_error(
    //         static_cast<int>(resp->errorcode()),
    //         resp->error().empty() ? "Set propertys failed" : resp->error()
    //     ));
    // }
    
    // // 成功请求后更新本地数据库和缓存
    // auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    // bool update_result = co_await msg_ds->UpdateMessagePropertysStatus(CTX_V, msg_id, propertys);
    // if (!update_result) {
    //     co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    // }
    
    co_return true;
}

/// 设置消息本地扩展字段（仅本地，不发送网络请求）
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetLocalExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // // 仅更新本地数据库和缓存，不发送网络请求
    // auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    // bool update_result = co_await msg_ds->UpdateMessageLocalExtStatus(CTX_V, msg_id, local_ext);
    // if (!update_result) {
    //     co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    // }
    
    co_return true;
}

/// 删除消息
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::Delete(CTX_T, std::string msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // auto cmd_msg = std::make_unique<network::CmdMessage>();
    // cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_DELETE));
    // auto* msg_data = cmd_msg->mutable_message();
    // msg_data->set_cmessaegid(msg_id);
    // msg_data->set_isdeleted(true);
    
    // auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    // if (!resp) {
    //     co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    // }
    
    // // 检查响应错误码
    // if (resp->errorcode() != 0) {
    //     co_return std::unexpected(roc::error::make_error(
    //         static_cast<int>(resp->errorcode()),
    //         resp->error().empty() ? "Delete message failed" : resp->error()
    //     ));
    // }
    
    // // 成功请求后更新本地数据库和缓存
    // auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    // bool update_result = co_await msg_ds->UpdateMessageDeletedStatus(CTX_V, msg_id, true);
    // if (!update_result) {
    //     co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    // }
    
    co_return true;
}

/// 撤回消息
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::Recall(CTX_T, std::string msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // auto cmd_msg = std::make_unique<network::CmdMessage>();
    // cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_RECALL));
    // auto* msg_data = cmd_msg->mutable_message();
    // msg_data->set_cmessaegid(msg_id);
    // msg_data->set_isrecalled(true);
    
    // auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    // if (!resp) {
    //     co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    // }
    
    // // 检查响应错误码
    // if (resp->errorcode() != 0) {
    //     co_return std::unexpected(roc::error::make_error(
    //         static_cast<int>(resp->errorcode()),
    //         resp->error().empty() ? "Recall message failed" : resp->error()
    //     ));
    // }
    
    // // 成功请求后更新本地数据库和缓存
    // auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    // bool update_result = co_await msg_ds->UpdateMessageRecalledStatus(CTX_V, msg_id, true);
    // if (!update_result) {
    //     co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    // }
    
    co_return true;
}

// ================================ handler ===============================

boost::asio::awaitable<void> MessageStatusHandler::p_onPinChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // // 更新数据库和缓存
    // auto msg_manager = sdk_root->MessageManager();
    // auto msg_ds = msg_manager->message_data_source.get();
    // co_await msg_ds->UpdateMessagePinStatus(CTX_V, cmd->message().cmessaegid(), cmd->message().ispinned());
    
    // // 获取更新后的消息并触发用户回调
    // auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->message().cmessaegid());
    // if (sdk_msg) {
    //     model::OnMessageResult result{};
    //     result.pin_changed_msgs.push_back(sdk_msg);
    //     base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    // }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // // 解析 sync_ext string 到 map
    // std::string sync_ext_str = cmd->message().syncext();
    // auto parse_result = json_util::MapParseFromString(sync_ext_str);
    // if (!parse_result) {
    //     LOG_INFO("MsgStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
    //     co_return;
    // }
    // std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

    // // 更新数据库和缓存
    // auto msg_manager = sdk_root->MessageManager();
    // auto msg_ds = msg_manager->message_data_source.get();
    // co_await msg_ds->UpdateMessageSyncExtStatus(CTX_V, cmd->message().cmessaegid(), sync_ext_map);
    
    // // 获取更新后的消息并触发用户回调
    // auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->message().cmessaegid());
    // if (sdk_msg) {
    //     model::OnMessageResult result{};
    //     result.sync_ext_changed_msgs.push_back(sdk_msg);
    //     base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    // }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onPropertyChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // // 解析 propertys string 到 vector
    // std::string propertys_str = cmd->message().propertys();
    // auto parse_result = json_util::Int32VectorParseFromString(propertys_str);
    // if (!parse_result) {
    //     LOG_INFO("MsgStatusHandler", "Failed to parse propertys: {}", parse_result.error().to_string());
    //     co_return;
    // }
    // std::vector<int32_t> propertys_vec = parse_result.value();

    // // 更新数据库和缓存
    // auto msg_manager = sdk_root->MessageManager();
    // auto msg_ds = msg_manager->message_data_source.get();
    // co_await msg_ds->UpdateMessagePropertysStatus(CTX_V, cmd->message().cmessaegid(), propertys_vec);
    
    // // 获取更新后的消息并触发用户回调
    // auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->message().cmessaegid());
    // if (sdk_msg) {
    //     model::OnMessageResult result{};
    //     result.property_changed_msgs.push_back(sdk_msg);
    //     base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    // }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // std::string msg_id = cmd->message().cmessaegid();
    // auto msg_manager = sdk_root->MessageManager();
    // auto msg_ds = msg_manager->message_data_source.get();

    // // 更新数据库和缓存
    // co_await msg_ds->UpdateMessageDeletedStatus(CTX_V, msg_id, true);

    // // 获取更新后的消息并触发用户回调
    // auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, msg_id);
    // if (sdk_msg) {
    //     model::OnMessageResult result{};
    //     result.deleted_msgs.push_back(sdk_msg);
    //     base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    // }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onRecall(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // std::string msg_id = cmd->message().cmessaegid();
    // auto msg_manager = sdk_root->MessageManager();
    // auto msg_ds = msg_manager->message_data_source.get();

    // // 更新数据库和缓存
    // co_await msg_ds->UpdateMessageRecalledStatus(CTX_V, msg_id, true);

    // // 获取更新后的消息并触发用户回调
    // auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, msg_id);
    // if (sdk_msg) {
    //     model::OnMessageResult result{};
    //     result.recalled_msgs.push_back(sdk_msg);
    //     base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    // }
}


// =================================  register =================================

void MessageStatusHandler::p_registPinHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::MSG_PIN_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onPinChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registSyncExtHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::MSG_SYNC_EXT_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onSyncExtChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registPropertyHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::MSG_PROPERTY_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onPropertyChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registDeleteHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::MSG_DELETE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onDelete(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registRecallHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::MSG_RECALL),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onRecall(CTX_V, cmd);
        }
    );
}

// boost::asio::awaitable<std::unique_ptr<network::CmdMessageOptResult>>
//     MessageStatusHandler::p_request(CTX_T, std::unique_ptr<network::CmdMessage> cmd_msg)
// {
//     CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
//     std::unique_ptr<network::BatchChangeMessagesRequest> req = std::make_unique<network::BatchChangeMessagesRequest>();
//     req->mutable_cmdmessages()->AddAllocated(cmd_msg.release());

//     auto frontier_msg = std::make_unique<network::FrontierMessage>();
//     frontier_msg->service = common::SDKWSService;
//     frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::MESSAGE_CHANGE));
//     int payload_size = static_cast<int>(req->ByteSizeLong());
//     frontier_msg->payload.resize(payload_size);
//     req->SerializeToArray(frontier_msg->payload.data(), payload_size);

//     auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));

//     if (!response.has_value()) {
//         co_return nullptr;
//     }

//     std::unique_ptr<network::BatchChangeMessagesResponse> resp = std::make_unique<network::BatchChangeMessagesResponse>();
//     resp->ParseFromArray(response.value()->payload.data(), static_cast<int>(response.value()->payload.size()));
//     if (resp->results_size() <= 0) {
//         co_return nullptr;
//     }

//     std::unique_ptr<network::CmdMessageOptResult> resp_item(resp->mutable_results()->ReleaseLast());
//     co_return resp_item;
// }

}; // roc::imsdk::core::message

