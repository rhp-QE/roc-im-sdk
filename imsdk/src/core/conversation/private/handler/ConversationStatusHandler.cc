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
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_TOP_CHANGED));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_istop(is_top);
    
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
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_MUTE_CHANGE));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_ismuted(is_muted);
    
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
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_BLOCK_CHANGE));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_isblocked(is_blocked);
    
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
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_SYNC_EXT_CHANGED));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_syncext(sync_ext_str);
    
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
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_DELETE));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_isdelete(true);
    
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

/// 创建群聊
boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>> 
ConversationStatusHandler::CreateGroup(CTX_T, const model::CreateGroupContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_CREATE));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_ownerid(context.owner_user_id);
    conv_info->set_name(context.group_name);
    
    auto members_result = json_util::StringVectorSerializeAsString(context.member_user_ids);
    if (!members_result) {
        co_return std::unexpected(roc::error::make_error(3005, "Failed to serialize member list"));
    }
    conv_info->set_members(members_result.value());
    
    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Create group failed" : resp->error()
        ));
    }
    
    // BatchChangeConversationsResponse 不包含创建的会话，需通过 CMD 推送获取。暂返回协议不支持错误
    co_return std::unexpected(roc::error::make_error(3006, "Create group: new protocol does not return conversation in response, need CMD push"));
}

/// 邀请群成员
boost::asio::awaitable<std::expected<bool, roc::error::Error>> 
ConversationStatusHandler::InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    auto cmd_msg = std::make_unique<network::CmdMessage>();
    cmd_msg->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_INVITE));
    auto* conv_info = cmd_msg->mutable_conversation();
    conv_info->set_convid(context.conv_id);
    
    auto members_result = json_util::StringVectorSerializeAsString(context.member_user_ids);
    if (!members_result) {
        co_return std::unexpected(roc::error::make_error(3005, "Failed to serialize member list"));
    }
    conv_info->set_members(members_result.value());
    
    auto resp = co_await p_request(CTX_V, std::move(cmd_msg));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->error().empty() ? "Invite group members failed" : resp->error()
        ));
    }
    
    // 邀请群成员成功后，被邀请者会收到 CMD 消息（CONV_GROUP_INVITE），由 p_onGroupInvite 处理
    // 邀请者不需要额外处理，直接返回成功
    co_return true;
}

// ================================ handler ===============================

boost::asio::awaitable<void> ConversationStatusHandler::p_onTopOnChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationTopStatus(CTX_V, cmd->conversation().convid(), cmd->conversation().istop());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->conversation().convid());

    // 用户回调
    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->top_on_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onMuteChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationMuteStatus(CTX_V, cmd->conversation().convid(), cmd->conversation().ismuted());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->conversation().convid());

    // 用户回调
    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->mute_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onBlockChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationBlockStatus(CTX_V, cmd->conversation().convid(), cmd->conversation().isblocked());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->conversation().convid());

    // 用户回调
    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->block_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 解析 sync_ext string 到 map
    std::string sync_ext_str = cmd->conversation().syncext();
    auto parse_result = json_util::MapParseFromString(sync_ext_str);
    if (!parse_result) {
        LOG_INFO("ConvStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
        co_return;
    }
    std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationSyncExtStatus(CTX_V, cmd->conversation().convid(), sync_ext_map);
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->conversation().convid());

    // 用户回调
    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->sync_ext_change_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    bool is_deleted = cmd->conversation().isdelete();
    co_await conv_ds->UpdateConversationDeletedStatus(CTX_V, cmd->conversation().convid(), is_deleted);
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->conversation().convid());

    // 用户回调
    if (sdk_conv) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->deleted_convs.push_back(sdk_conv);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

boost::asio::awaitable<void> ConversationStatusHandler::p_onGroupInvite(CTX_T, std::shared_ptr<const network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 处理群聊邀请 CMD 消息（只有被邀请者会收到）
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    
    // 将 CMD 消息中的 ConversationData 转换为网络会话格式并保存
    auto net_conv = std::make_shared<network::ConversationData>();
    net_conv->CopyFrom(cmd->conversation());
    auto sdk_convs = co_await conv_ds->SaveNetConversations(CTX_V, {net_conv});

    // 用户回调
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