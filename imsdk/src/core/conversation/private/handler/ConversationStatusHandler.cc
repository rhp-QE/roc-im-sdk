#include "imsdk/src/core/conversation/private/handler/ConversationStatusHandler.h"
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
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/datasource/ConvDatasource.h"
#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/common/json_util.h"
#include "imsdk/src/include/model/network.h"

#include "imsdk/src/core/cmd/CmdCenter.h"

namespace roc::imsdk::core::conversation {

ConversationStatusHandler::ConversationStatusHandler(std::weak_ptr<SDKRoot> root) :
    w_sdk_root(root)
{}

void ConversationStatusHandler::AllComponentDidLoad(CTX_T) {
    // p_registBlockHandler(CTX_V);
    // p_registMuteHandler(CTX_V);
    // p_registTopOnHandler(CTX_V);
    // p_registSyncExtHandler(CTX_V);
    // p_registDeleteHandler(CTX_V);
}

/// 置顶设置
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetTopOn(CTX_T, std::string cid, bool is_top) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // network::ConversationData conv_data;
    // conv_data.set_convid(cid);
    // conv_data.set_istop(is_top);
    // std::string data_buf;
    // if (!conv_data.SerializeToString(&data_buf)) { ... }
    // auto cmd_msg = std::make_unique<network::CmdMessage>(); ...
    // auto resp = co_await p_request(CTX_V, std::move(cmd_msg)); ...
    // auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    // bool update_result = co_await conv_ds->UpdateConversationTopStatus(...); ...
    co_return true;
}
    
/// 免打扰
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetMute(CTX_T, std::string cid, bool is_muted) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // 实现已注释
    co_return true;
}

/// 拉黑
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetBlock(CTX_T, std::string cid, bool is_blocked) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // 实现已注释
    co_return true;
}

/// 设置同步扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetSyncExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // 实现已注释
    co_return true;
}

/// 设置本地扩展字段（仅本地，不发送网络请求）
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetLocalExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // 实现已注释
    co_return true;
}

/// 删除会话
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::Delete(CTX_T, std::string cid) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    // 实现已注释
    co_return true;
}

// ================================ handler ===============================

/// 从 CmdMessage.data 解析出 ConversationData，解析失败返回 nullptr
static std::shared_ptr<network::ConversationData> ParseConvDataFromCmd(const network::CmdMessage& cmd) {
    // if (cmd.data().empty()) { return nullptr; }
    // auto conv = std::make_shared<network::ConversationData>();
    // if (!conv->ParseFromArray(cmd.data().data(), static_cast<int>(cmd.data().size()))) { return nullptr; }
    // return conv;
    return nullptr;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onTopOnChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    // 实现已注释
    (void)cmd;
    co_return;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onMuteChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    // 实现已注释
    (void)cmd;
    co_return;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onBlockChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    // 实现已注释
    (void)cmd;
    co_return;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    // 实现已注释
    (void)cmd;
    co_return;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
    // 实现已注释
    (void)cmd;
    co_return;
}

// =================================  register =================================

void ConversationStatusHandler::p_registTopOnHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    // sdk_root->cmd_center()->RegistCmdHandler(CTX_V, CONV_TOP_CHANGED, ...);
}

void ConversationStatusHandler::p_registMuteHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    // sdk_root->cmd_center()->RegistCmdHandler(CTX_V, CONV_MUTE_CHANGE, ...);
}

void ConversationStatusHandler::p_registBlockHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    // sdk_root->cmd_center()->RegistCmdHandler(CTX_V, CONV_BLOCK_CHANGE, ...);
}

void ConversationStatusHandler::p_registSyncExtHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    // sdk_root->cmd_center()->RegistCmdHandler(CTX_V, CONV_SYNC_EXT_CHANGED, ...);
}

void ConversationStatusHandler::p_registDeleteHandler(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    // sdk_root->cmd_center()->RegistCmdHandler(CTX_V, CONV_DELETE, ...);
}


 
// boost::asio::awaitable<std::unique_ptr<network::CmdMessageOptResult>>
//     ConversationStatusHandler::p_request(CTX_T, std::unique_ptr<network::CmdMessage> cmd_msg)
// {
//     CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
//     std::unique_ptr<network::BatchChangeConversationsRequest> req = std::make_unique<network::BatchChangeConversationsRequest>();
//     req->mutable_cmdmessages()->AddAllocated(cmd_msg.release());

//     auto frontier_msg = std::make_unique<network::FrontierMessage>();
//     frontier_msg->service = common::SDKWSService;
//     frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::CONVERSATION_CHANGE));
//     int payload_size = static_cast<int>(req->ByteSizeLong());
//     frontier_msg->payload.resize(payload_size);
//     req->SerializeToArray(frontier_msg->payload.data(), payload_size);

//     auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));

//     if (!response.has_value()) {
//         co_return nullptr;
//     }

//     std::unique_ptr<network::BatchChangeConversationsResponse> resp = std::make_unique<network::BatchChangeConversationsResponse>();
//     resp->ParseFromArray(response.value()->payload.data(), static_cast<int>(response.value()->payload.size()));
//     if (resp->results_size() <= 0) {
//         co_return nullptr;
//     }

//     std::unique_ptr<network::CmdMessageOptResult> resp_item(resp->mutable_results()->ReleaseLast());
//     co_return resp_item;
// }

}; // roc::imsdk::core::conversation