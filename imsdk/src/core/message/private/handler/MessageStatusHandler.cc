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

void MessageStatusHandler::AllComponentDidLoad() {
    p_registPinHandler();
    p_registSyncExtHandler();
    p_registPropertyHandler();
    p_registDeleteHandler();
    p_registRecallHandler();
}

/// 设置消息置顶状态
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetPin(CTX_T, std::string msg_id, bool is_pinned) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 构造请求
    std::unique_ptr<network::ChangeMessageItemReq> req_item = std::make_unique<network::ChangeMessageItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_PIN_CHANGED));
    auto* msg_data = req_item->mutable_message();
    msg_data->set_clientmsgid(msg_id);
    msg_data->set_ispinned(is_pinned);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set pin failed" : resp->errormsg()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessagePinStatus(CTX_V, msg_id, is_pinned);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 设置消息同步扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetSyncExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 序列化 map 到 JSON string
    auto sync_ext_str_result = json_util::MapSerializeAsString(sync_ext);
    if (!sync_ext_str_result) {
        co_return std::unexpected(sync_ext_str_result.error());
    }
    std::string sync_ext_str = sync_ext_str_result.value();
    
    // 构造请求
    std::unique_ptr<network::ChangeMessageItemReq> req_item = std::make_unique<network::ChangeMessageItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_SYNC_EXT_CHANGED));
    auto* msg_data = req_item->mutable_message();
    msg_data->set_clientmsgid(msg_id);
    msg_data->set_syncext(sync_ext_str);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set sync_ext failed" : resp->errormsg()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessageSyncExtStatus(CTX_V, msg_id, sync_ext);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 设置消息属性
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetPropertys(CTX_T, std::string msg_id, const std::vector<int32_t> &propertys) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 序列化 vector 到 JSON string
    auto propertys_str_result = json_util::Int32VectorSerializeAsString(propertys);
    if (!propertys_str_result) {
        co_return std::unexpected(propertys_str_result.error());
    }
    std::string propertys_str = propertys_str_result.value();
    
    // 构造请求
    std::unique_ptr<network::ChangeMessageItemReq> req_item = std::make_unique<network::ChangeMessageItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_PROPERTY_CHANGED));
    auto* msg_data = req_item->mutable_message();
    msg_data->set_clientmsgid(msg_id);
    msg_data->set_propertys(propertys_str);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set propertys failed" : resp->errormsg()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessagePropertysStatus(CTX_V, msg_id, propertys);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 设置消息本地扩展字段（仅本地，不发送网络请求）
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::SetLocalExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 仅更新本地数据库和缓存，不发送网络请求
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessageLocalExtStatus(CTX_V, msg_id, local_ext);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 删除消息
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::Delete(CTX_T, std::string msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 构造请求
    std::unique_ptr<network::ChangeMessageItemReq> req_item = std::make_unique<network::ChangeMessageItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_DELETE));
    auto* msg_data = req_item->mutable_message();
    msg_data->set_clientmsgid(msg_id);
    msg_data->set_isdeleted(true);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Delete message failed" : resp->errormsg()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessageDeletedStatus(CTX_V, msg_id, true);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 撤回消息
boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageStatusHandler::Recall(CTX_T, std::string msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 构造请求
    std::unique_ptr<network::ChangeMessageItemReq> req_item = std::make_unique<network::ChangeMessageItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::MSG_RECALL));
    auto* msg_data = req_item->mutable_message();
    msg_data->set_clientmsgid(msg_id);
    msg_data->set_isrecalled(true);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Recall message failed" : resp->errormsg()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto msg_ds = sdk_root->MessageManager()->message_data_source.get();
    bool update_result = co_await msg_ds->UpdateMessageRecalledStatus(CTX_V, msg_id, true);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

// ================================ handler ===============================

boost::asio::awaitable<void> MessageStatusHandler::p_onPinChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 更新数据库和缓存
    auto msg_manager = sdk_root->MessageManager();
    auto msg_ds = msg_manager->message_data_source.get();
    co_await msg_ds->UpdateMessagePinStatus(CTX_V, cmd->msg().clientmsgid(), cmd->msg().ispinned());
    
    // 获取更新后的消息并触发用户回调
    auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->msg().clientmsgid());
    if (sdk_msg) {
        model::OnMessageResult result{};
        result.pin_changed_msgs.push_back(sdk_msg);
        base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 解析 sync_ext string 到 map
    std::string sync_ext_str = cmd->msg().syncext();
    auto parse_result = json_util::MapParseFromString(sync_ext_str);
    if (!parse_result) {
        LOG_INFO("MsgStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
        co_return;
    }
    std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

    // 更新数据库和缓存
    auto msg_manager = sdk_root->MessageManager();
    auto msg_ds = msg_manager->message_data_source.get();
    co_await msg_ds->UpdateMessageSyncExtStatus(CTX_V, cmd->msg().clientmsgid(), sync_ext_map);
    
    // 获取更新后的消息并触发用户回调
    auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->msg().clientmsgid());
    if (sdk_msg) {
        model::OnMessageResult result{};
        result.sync_ext_changed_msgs.push_back(sdk_msg);
        base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onPropertyChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 解析 propertys string 到 vector
    std::string propertys_str = cmd->msg().propertys();
    auto parse_result = json_util::Int32VectorParseFromString(propertys_str);
    if (!parse_result) {
        LOG_INFO("MsgStatusHandler", "Failed to parse propertys: {}", parse_result.error().to_string());
        co_return;
    }
    std::vector<int32_t> propertys_vec = parse_result.value();

    // 更新数据库和缓存
    auto msg_manager = sdk_root->MessageManager();
    auto msg_ds = msg_manager->message_data_source.get();
    co_await msg_ds->UpdateMessagePropertysStatus(CTX_V, cmd->msg().clientmsgid(), propertys_vec);
    
    // 获取更新后的消息并触发用户回调
    auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, cmd->msg().clientmsgid());
    if (sdk_msg) {
        model::OnMessageResult result{};
        result.property_changed_msgs.push_back(sdk_msg);
        base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    std::string msg_id = cmd->msg().clientmsgid();
    auto msg_manager = sdk_root->MessageManager();
    auto msg_ds = msg_manager->message_data_source.get();

    // 更新数据库和缓存
    co_await msg_ds->UpdateMessageDeletedStatus(CTX_V, msg_id, true);

    // 获取更新后的消息并触发用户回调
    auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, msg_id);
    if (sdk_msg) {
        model::OnMessageResult result{};
        result.deleted_msgs.push_back(sdk_msg);
        base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    }
}

boost::asio::awaitable<void> MessageStatusHandler::p_onRecall(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    std::string msg_id = cmd->msg().clientmsgid();
    auto msg_manager = sdk_root->MessageManager();
    auto msg_ds = msg_manager->message_data_source.get();

    // 更新数据库和缓存
    co_await msg_ds->UpdateMessageRecalledStatus(CTX_V, msg_id, true);

    // 获取更新后的消息并触发用户回调
    auto sdk_msg = co_await msg_ds->SdkMsgForId(CTX_V, msg_id);
    if (sdk_msg) {
        model::OnMessageResult result{};
        result.recalled_msgs.push_back(sdk_msg);
        base::util::safe_invoke_block(msg_manager->OnMessagesCallback(), result);
    }
}


// =================================  register =================================

void MessageStatusHandler::p_registPinHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::MSG_PIN_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onPinChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registSyncExtHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::MSG_SYNC_EXT_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onSyncExtChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registPropertyHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::MSG_PROPERTY_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onPropertyChange(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registDeleteHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::MSG_DELETE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onDelete(CTX_V, cmd);
        }
    );
}

void MessageStatusHandler::p_registRecallHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::MSG_RECALL),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onRecall(CTX_V, cmd);
        }
    );
}

boost::asio::awaitable<std::unique_ptr<network::ChangeMessageItemResp>>
    MessageStatusHandler::p_request(CTX_T, std::unique_ptr<network::ChangeMessageItemReq> req_item)
{
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    std::unique_ptr<network::ChangeMessageReq> req = std::make_unique<network::ChangeMessageReq>();
    req->mutable_infos()->AddAllocated(req_item.release());

    // 创建 FrontierMessage 请求
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::MESSAGE_CHANGE));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = req->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    req->SerializeToArray(frontier_msg->payload.data(), payload_size);
    frontier_msg->metadata["track_id"] = std::to_string(call_track_id);

    auto response = co_await sdk_root->ConnectionManager()->SendRequest(std::move(frontier_msg));

    if (!response.has_value()) {
        co_return nullptr;
    }

    // 从响应的 payload 中解析 ChangeMessageResp
    std::unique_ptr<network::ChangeMessageResp> resp = std::make_unique<network::ChangeMessageResp>();
    // 直接使用 vector 中的数据解析，避免拷贝
    resp->ParseFromArray(response.value()->payload.data(), response.value()->payload.size());
    if (resp->infos().size() <= 0) {
        co_return nullptr;
    }

    std::unique_ptr<network::ChangeMessageItemResp> resp_item = std::unique_ptr<network::ChangeMessageItemResp>(resp->mutable_infos()->ReleaseLast());
    co_return resp_item;
}

}; // roc::imsdk::core::message

