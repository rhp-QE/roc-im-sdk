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
#include <boost/json.hpp>

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
    
/// 禁言
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
    auto sync_ext_str_result = util::MapSerializeAsString(sync_ext);
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



// ================================ private ===============================


void ConversationStatusHandler::p_registTopOnHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_TOP_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

            // 更新数据库
            auto conv_manager = sdk_root->ConversationManager();
            auto conv_ds = conv_manager->conv_datasource.get();
            co_await conv_ds->UpdateConversationTopStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().istop());
            auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

            // 用户回调
            if (sdk_conv) {
                auto on_conversation_result = std::make_shared<model::OnConversationResult>();
                on_conversation_result->top_on_change_convs.push_back(sdk_conv);
                base::util::safe_invoke_block(conv_manager->OnConvUpdateCallback(), on_conversation_result);
            }
        }
    );
}

void ConversationStatusHandler::p_registMuteHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_MUTE_CHANGE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

            // 更新数据库
            auto conv_manager = sdk_root->ConversationManager();
            auto conv_ds = conv_manager->conv_datasource.get();
            co_await conv_ds->UpdateConversationMuteStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().ismuted());
            auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

            // 用户回调
            if (sdk_conv) {
                auto on_conversation_result = std::make_shared<model::OnConversationResult>();
                on_conversation_result->mute_change_convs.push_back(sdk_conv);
                base::util::safe_invoke_block(conv_manager->OnConvUpdateCallback(), on_conversation_result);
            }
        }
    );
}

void ConversationStatusHandler::p_registBlockHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_BLOCK_CHANGE),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

            // 更新数据库
            auto conv_manager = sdk_root->ConversationManager();
            auto conv_ds = conv_manager->conv_datasource.get();
            co_await conv_ds->UpdateConversationBlockStatus(CTX_V, cmd->convinfo().convid(), cmd->convinfo().isblocked());
            auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

            // 用户回调
            if (sdk_conv) {
                auto on_conversation_result = std::make_shared<model::OnConversationResult>();
                on_conversation_result->block_change_convs.push_back(sdk_conv);
                base::util::safe_invoke_block(conv_manager->OnConvUpdateCallback(), on_conversation_result);
            }
        }
    );
}

void ConversationStatusHandler::p_registSyncExtHandler() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        static_cast<int32_t>(common::CmdMessageOp::CONV_SYNC_EXT_CHANGED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void>{
            CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

            // 解析 sync_ext string 到 map
            std::string sync_ext_str = cmd->convinfo().syncext();
            auto parse_result = util::MapParseFromString(sync_ext_str);
            if (!parse_result) {
                LOG_INFO("ConvStatusHandler", "Failed to parse sync_ext: {}", parse_result.error().to_string());
                co_return;
            }
            std::unordered_map<std::string, std::string> sync_ext_map = parse_result.value();

            // 更新数据库
            auto conv_manager = sdk_root->ConversationManager();
            auto conv_ds = conv_manager->conv_datasource.get();
            co_await conv_ds->UpdateConversationSyncExtStatus(CTX_V, cmd->convinfo().convid(), sync_ext_map);
            auto sdk_conv = co_await conv_ds->SdkConvForId(CTX_V, cmd->convinfo().convid());

            // 用户回调
            if (sdk_conv) {
                auto on_conversation_result = std::make_shared<model::OnConversationResult>();
                on_conversation_result->sync_ext_change_convs.push_back(sdk_conv);
                base::util::safe_invoke_block(conv_manager->OnConvUpdateCallback(), on_conversation_result);
            }
        }
    );
}

 
boost::asio::awaitable<std::unique_ptr<network::ChangeConversationItemResp>>
    ConversationStatusHandler::p_request(CTX_T, std::unique_ptr<network::ChangeConversationItemReq> req_item)
{
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    std::unique_ptr<network::ChangeConversationReq> req = std::make_unique<network::ChangeConversationReq>();
    req->mutable_infos()->AddAllocated(req_item.release());

    std::unique_ptr<network::SdkWSReq> request = std::make_unique<network::SdkWSReq>();
    request->set_data(req->SerializeAsString());
    request->set_service(common::SDKWSService);
    request->set_method(static_cast<uint32_t>(common::SDKWSMethod::CONVERSATION_CHANGE));
    request->set_trackid(call_track_id);

    auto response = co_await sdk_root->ConnectionManager()->SendRequest(request.get());

    if (!response) {
        co_return nullptr;
    }

    std::unique_ptr<network::ChangeConversationResp> resp = std::make_unique<network::ChangeConversationResp>();
    resp->ParseFromString(response.value()->data());
    if (resp->infos().size() <= 0) {
        co_return nullptr;
    }

    std::unique_ptr<network::ChangeConversationItemResp> resp_item = std::unique_ptr<network::ChangeConversationItemResp>(resp->mutable_infos()->ReleaseLast());
    co_return resp_item;
}

}; // roc::imsdk::core::conversation