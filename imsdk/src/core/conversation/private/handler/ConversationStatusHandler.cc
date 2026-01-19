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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_TOP_CHANGED));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_istop(is_top);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set top failed" : resp->errormsg()
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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_MUTE_CHANGE));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_ismuted(is_muted);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set mute failed" : resp->errormsg()
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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_BLOCK_CHANGE));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_isblocked(is_blocked);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Set block failed" : resp->errormsg()
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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_SYNC_EXT_CHANGED));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_syncext(sync_ext_str);
    
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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_DELETE));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(cid);
    conv_info->set_isdelete(true);
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Delete conversation failed" : resp->errormsg()
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
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_CREATE));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_owneruserid(context.owner_user_id);
    conv_info->set_convname(context.group_name);
    
    // 序列化成员列表为 JSON 字符串
    auto members_result = json_util::StringVectorSerializeAsString(context.member_user_ids);
    if (!members_result) {
        co_return std::unexpected(roc::error::make_error(3005, "Failed to serialize member list"));
    }
    conv_info->set_members(members_result.value());
    
    // 发送网络请求并使用 p_request 函数
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Create group failed" : resp->errormsg()
        ));
    }
    
    // 从响应中获取会话信息（响应中的 convInfo 字段）
    if (!resp->has_convinfo()) {
        co_return std::unexpected(roc::error::make_error(3006, "Create group: response does not contain conversation info"));
    }
    
    // 将响应中的会话信息转换为网络会话格式并保存
    auto conv_ds = sdk_root->ConversationManager()->conv_datasource.get();
    auto net_conv = std::shared_ptr<network::ConversationInfo>(resp->release_convinfo());
    auto sdk_convs = co_await conv_ds->SaveNetConversations(CTX_V, {net_conv});
    
    if (sdk_convs.empty()) {
        co_return std::unexpected(roc::error::make_error(3007, "Create group: failed to save conversation"));
    }
    
    co_return sdk_convs[0];
}

/// 邀请群成员
boost::asio::awaitable<std::expected<bool, roc::error::Error>> 
ConversationStatusHandler::InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 构造请求
    std::unique_ptr<network::ChangeConversationItemReq> req_item = std::make_unique<network::ChangeConversationItemReq>();
    req_item->set_cmd(static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_INVITE));
    auto* conv_info = req_item->mutable_conversation();
    conv_info->set_convid(context.conv_id);
    
    // 序列化成员列表为 JSON 字符串
    auto members_result = json_util::StringVectorSerializeAsString(context.member_user_ids);
    if (!members_result) {
        co_return std::unexpected(roc::error::make_error(3005, "Failed to serialize member list"));
    }
    conv_info->set_members(members_result.value());
    
    // 发送网络请求
    auto resp = co_await p_request(CTX_V, std::move(req_item));
    if (!resp) {
        co_return std::unexpected(roc::error::make_error(3003, "Network request failed"));
    }
    
    // 检查响应错误码
    if (resp->errorcode() != 0) {
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp->errorcode()),
            resp->errormsg().empty() ? "Invite group members failed" : resp->errormsg()
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
    co_await conv_ds->UpdateConversationTopStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().istop());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

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
    co_await conv_ds->UpdateConversationMuteStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().ismuted());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

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
    co_await conv_ds->UpdateConversationBlockStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().isblocked());
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

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
    std::string sync_ext_str = cmd->convinfo().syncext();
    auto parse_result = json_util::MapParseFromString(sync_ext_str);
    if (!parse_result) {
        LOG_INFO("ConvStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
        co_return;
    }
    std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

    // 更新数据库和缓存
    auto conv_manager = sdk_root->ConversationManager();
    auto conv_ds = conv_manager->conv_datasource.get();
    co_await conv_ds->UpdateConversationSyncExtStatus(CTX_V, cmd->convinfo().convid(), sync_ext_map);
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

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
    bool is_deleted = cmd->convinfo().isdelete();
    co_await conv_ds->UpdateConversationDeletedStatus(CTX_V, cmd->convinfo().convid(), is_deleted);
    auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

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
    
    // 将 CMD 消息中的 ConversationInfo 转换为网络会话格式并保存
    auto net_conv = std::make_shared<network::ConversationInfo>();
    net_conv->CopyFrom(cmd->convinfo());
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

 
boost::asio::awaitable<std::unique_ptr<network::ChangeConversationItemResp>>
    ConversationStatusHandler::p_request(CTX_T, std::unique_ptr<network::ChangeConversationItemReq> req_item)
{
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    std::unique_ptr<network::ChangeConversationReq> req = std::make_unique<network::ChangeConversationReq>();
    req->mutable_infos()->AddAllocated(req_item.release());

    // 创建 FrontierMessage 请求
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::CONVERSATION_CHANGE));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = req->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    req->SerializeToArray(frontier_msg->payload.data(), payload_size);
    frontier_msg->metadata["track_id"] = std::to_string(call_track_id);

    auto response = co_await sdk_root->ConnectionManager()->SendRequest(std::move(frontier_msg));

    if (!response.has_value()) {
        co_return nullptr;
    }

    // 从响应的 payload 中解析 ChangeConversationResp
    std::unique_ptr<network::ChangeConversationResp> resp = std::make_unique<network::ChangeConversationResp>();
    // 直接使用 vector 中的数据解析，避免拷贝
    resp->ParseFromArray(response.value()->payload.data(), response.value()->payload.size());
    if (resp->infos().size() <= 0) {
        co_return nullptr;
    }

    std::unique_ptr<network::ChangeConversationItemResp> resp_item = std::unique_ptr<network::ChangeConversationItemResp>(resp->mutable_infos()->ReleaseLast());
    co_return resp_item;
}

}; // roc::imsdk::core::conversation