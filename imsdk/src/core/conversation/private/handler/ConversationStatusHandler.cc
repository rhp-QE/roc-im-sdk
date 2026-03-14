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

void ConversationStatusHandler::AllComponentDidLoad() {
    p_registBlockHandler();
    p_registMuteHandler();
    p_registTopOnHandler();
    p_registSyncExtHandler();
    p_registDeleteHandler();
    p_registGroupInviteHandler();
}

/// 置顶设置
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetTopOn(CTX_T, std::string cid, bool is_top) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::ConversationData conv_data;
    conv_data.set_convid(cid);
    conv_data.set_istop(is_top);
    std::string data_buf;
    if (!conv_data.SerializeToString(&data_buf)) {
        co_return std::unexpected(roc::error::make_error(3005, "SetTopOn: serialize failed"));
    }
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_TOP_CHANGED));
    cmd_msg->set_data(data_buf);

    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Set top failed" : resp->error()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationTopStatus(CTX_V, cid, is_top);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}
    
/// 免打扰
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetMute(CTX_T, std::string cid, bool is_muted) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::ConversationData conv_data;
    conv_data.set_convid(cid);
    conv_data.set_ismuted(is_muted);
    std::string data_buf;
    if (!conv_data.SerializeToString(&data_buf)) {
        co_return std::unexpected(roc::error::make_error(3005, "SetMute: serialize failed"));
    }
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_MUTE_CHANGE));
    cmd_msg->set_data(data_buf);

    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Set mute failed" : resp->error()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationMuteStatus(CTX_V, cid, is_muted);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 拉黑
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetBlock(CTX_T, std::string cid, bool is_blocked) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::ConversationData conv_data;
    conv_data.set_convid(cid);
    conv_data.set_isblocked(is_blocked);
    std::string data_buf;
    if (!conv_data.SerializeToString(&data_buf)) {
        co_return std::unexpected(roc::error::make_error(3005, "SetBlock: serialize failed"));
    }
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_BLOCK_CHANGE));
    cmd_msg->set_data(data_buf);

    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Set block failed" : resp->error()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationBlockStatus(CTX_V, cid, is_blocked);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 设置同步扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetSyncExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 序列化 map 到 JSON string
    auto sync_ext_str_result = json_util::MapSerializeAsString(sync_ext);
    if (!sync_ext_str_result) {
        co_return std::unexpected(sync_ext_str_result.error());
    }
    std::string sync_ext_str = sync_ext_str_result.value();

    network::ConversationData conv_data;
    conv_data.set_convid(cid);
    conv_data.set_syncext(sync_ext_str);
    std::string data_buf;
    if (!conv_data.SerializeToString(&data_buf)) {
        co_return std::unexpected(roc::error::make_error(3005, "SetSyncExt: serialize failed"));
    }
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_SYNC_EXT_CHANGED));
    cmd_msg->set_data(data_buf);

    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Set sync_ext failed" : resp->error()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationSyncExtStatus(CTX_V, cid, sync_ext);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 设置本地扩展字段（仅本地，不发送网络请求）
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::SetLocalExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 直接更新本地数据库和缓存，不发送网络请求
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationLocalExtStatus(CTX_V, cid, local_ext);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

/// 删除会话
boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationStatusHandler::Delete(CTX_T, std::string cid) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::ConversationData conv_data;
    conv_data.set_convid(cid);
    conv_data.set_isdelete(true);
    std::string data_buf;
    if (!conv_data.SerializeToString(&data_buf)) {
        co_return std::unexpected(roc::error::make_error(3005, "Delete: serialize failed"));
    }
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_DELETE));
    cmd_msg->set_data(data_buf);

    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Delete conversation failed" : resp->error()
        ));
    }
    
    // 成功请求后更新本地数据库和缓存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    bool update_result = co_await conv_ds->UpdateConversationDeletedStatus(CTX_V, cid, true);
    if (!update_result) {
        co_return std::unexpected(roc::error::make_error(3004, "Failed to update local database"));
    }
    
    co_return true;
}

// ================================ handler ===============================

/// 从 CmdMessage.data 解析出 ConversationData，解析失败返回 nullptr
static std::shared_ptr<network::ConversationData> ParseConvDataFromCmd(const network::CmdMessage& cmd) {
    if (cmd.data().empty()) {
        return nullptr;
    }
    auto conv = std::make_shared<network::ConversationData>();
    if (!conv->ParseFromArray(cmd.data().data(), static_cast<int>(cmd.data().size()))) {
        return nullptr;
    }
    return conv;
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onTopOnChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto conv = ParseConvDataFromCmd(*cmd);
    if (!conv) {
        LOG_INFO("ConvStatusHandler", "p_onTopOnChange: parse data failed");
        co_return;
    }
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationTopStatus(CTX_V, conv->convid(), conv->istop());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, conv->convid());

    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->top_on_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onMuteChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto conv = ParseConvDataFromCmd(*cmd);
    if (!conv) {
        LOG_INFO("ConvStatusHandler", "p_onMuteChange: parse data failed");
        co_return;
    }
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationMuteStatus(CTX_V, conv->convid(), conv->ismuted());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, conv->convid());

    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->mute_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onBlockChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto conv = ParseConvDataFromCmd(*cmd);
    if (!conv) {
        LOG_INFO("ConvStatusHandler", "p_onBlockChange: parse data failed");
        co_return;
    }
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationBlockStatus(CTX_V, conv->convid(), conv->isblocked());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, conv->convid());

    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->block_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto conv = ParseConvDataFromCmd(*cmd);
    if (!conv) {
        LOG_INFO("ConvStatusHandler", "p_onSyncExtChange: parse data failed");
        co_return;
    }
    std::string sync_ext_str = conv->syncext();
    auto parse_result = json_util::MapParseFromString(sync_ext_str);
    if (!parse_result) {
        LOG_INFO("ConvStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
        co_return;
    }
    std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationSyncExtStatus(CTX_V, conv->convid(), sync_ext_map);
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, conv->convid());

    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->sync_ext_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto conv = ParseConvDataFromCmd(*cmd);
    if (!conv) {
        LOG_INFO("ConvStatusHandler", "p_onDelete: parse data failed");
        co_return;
    }
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationDeletedStatus(CTX_V, conv->convid(), conv->isdelete());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, conv->convid());

    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->deleted_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onGroupInvite(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    auto net_conv = ParseConvDataFromCmd(*cmd);
    if (!net_conv) {
        LOG_INFO("ConvStatusHandler", "p_onGroupInvite: parse data failed");
        co_return;
    }
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    auto sdk_convs = co_await conv_ds->SaveNetConversations(CTX_V, {net_conv});

    if (!sdk_convs.empty()) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->invited_convs.push_back(sdk_convs[0]);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

// =================================  register =================================

void ConversationStatusHandler::p_registTopOnHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_TOP_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onTopOnChange(CTX_V, cmd);
        }
    );
}

void ConversationStatusHandler::p_registMuteHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_MUTE_CHANGE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onMuteChange(CTX_V, cmd);
        }
    );
}

void ConversationStatusHandler::p_registBlockHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_BLOCK_CHANGE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onBlockChange(CTX_V, cmd);
        }
    );
}

void ConversationStatusHandler::p_registSyncExtHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_SYNC_EXT_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onSyncExtChange(CTX_V, cmd);
        }
    );
}

void ConversationStatusHandler::p_registDeleteHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_DELETE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onDelete(CTX_V, cmd);
        }
    );
}


void ConversationStatusHandler::p_registGroupInviteHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_INVITE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            return p_onGroupInvite(CTX_V, cmd);
        }
    );
}

 
boost::asio::awaitable<std::unique_ptr<network::CmdMessageOptResult>>
    ConversationStatusHandler::p_request(CTX_T, std::unique_ptr<network::CmdMessage> cmd_msg)
{
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    std::unique_ptr<network::BatchChangeConversationsRequest> req = std::make_unique<network::BatchChangeConversationsRequest>();
    req->mutable_cmdmessages()->AddAllocated(cmd_msg.release());

    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::CONVERSATION_CHANGE));
    int payload_size = static_cast<int>(req->ByteSizeLong());
    frontier_msg->payload.resize(payload_size);
    req->SerializeToArray(frontier_msg->payload.data(), payload_size);

    auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));

    if (!response.has_value()) {
        co_return nullptr;
    }

    std::unique_ptr<network::BatchChangeConversationsResponse> resp = std::make_unique<network::BatchChangeConversationsResponse>();
    resp->ParseFromArray(response.value()->payload.data(), static_cast<int>(response.value()->payload.size()));
    if (resp->results_size() <= 0) {
        co_return nullptr;
    }

    std::unique_ptr<network::CmdMessageOptResult> resp_item(resp->mutable_results()->ReleaseLast());
    co_return resp_item;
}

}; // roc::imsdk::core::conversation