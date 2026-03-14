#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/datasource/ConvDatasource.h"
#include "imsdk/src/core/conversation/private/receive/ReceiveConversation.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"
#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"
#include "imsdk/src/core/conversation/private/handler/ConversationStatusHandler.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include "imsdk/src/core/common/logger_macro.h"

namespace roc::imsdk::core {

ConversationManager::ConversationManager(std::shared_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root)
{
    p_InitSubComponents();
}

ConversationManager::~ConversationManager() = default;

void ConversationManager::AllComponentDidLoad(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    /// 创建数据库表
    db_opt->CreateConversationTableIfNeed(CTX_V);

    conversation_status_handler->AllComponentDidLoad(CTX_V);
}

model::OnConversationsCallbackTy& ConversationManager::OnConversationsCallback() {
    return on_convs_callback_;
}

// =============================  conversation api implementations  ======================================

void ConversationManager::OnConvUpdate(model::OnConversationsCallbackTy callback) {
    on_convs_callback_ = callback;
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConversationManager::ConvForId(std::string conv_id) {
    START_TRACK;
    co_return co_await conv_datasource->SdkConvForId(CTX_V, conv_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::ConvsForUserId(std::string user_id, int64_t cursor, int64_t limit) {
    START_TRACK;
    co_return co_await conv_datasource->LoadConvsFromDb(CTX_V, cursor, limit, true);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::ConvsWhenLogin() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    START_TRACK;

    /// 触发混链拉取
    asio::co_spawn(sdk_root->net_io_context(), user_message_fetcher->FetchUserMessages(CTX_V), asio::detached);

    /// 从DB 中加载会话
    auto convs = co_await conv_datasource->LoadConvsFromDb(CTX_V, -1, 100, true);
    co_return convs;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::SetConvTop(std::string conv_id, bool is_top) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await conversation_status_handler->SetTopOn(CTX_V, conv_id, is_top);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::SetConvMute(std::string conv_id, bool is_mute) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await conversation_status_handler->SetMute(CTX_V, conv_id, is_mute);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::SetConvBlock(std::string conv_id, bool is_block) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await conversation_status_handler->SetBlock(CTX_V, conv_id, is_block);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::SetConvSyncExt(std::string conv_id, std::string key, std::string value) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    std::unordered_map<std::string, std::string> sync_ext;
    sync_ext[key] = value;
    co_return co_await conversation_status_handler->SetSyncExt(CTX_V, conv_id, sync_ext);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::SetConvLocalExt(std::string conv_id, std::string key, std::string value) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    std::unordered_map<std::string, std::string> local_ext;
    local_ext[key] = value;
    co_return co_await conversation_status_handler->SetLocalExt(CTX_V, conv_id, local_ext);
}

// 设置会话已读
boost::asio::awaitable<bool> ConversationManager::SetConvRead(std::string conv_id) {
    // TODO: Implement set conversation read
    co_return false;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> ConversationManager::DeleteConv(std::string conv_id) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await conversation_status_handler->Delete(CTX_V, conv_id);
}

boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>>
ConversationManager::SaveNetConversations(std::vector<std::shared_ptr<network::ConversationData>> convs) {
    START_TRACK;
    co_return co_await conv_datasource->SaveNetConversations(CTX_V, std::move(convs));
}

/// =======================================================================================

/// ================================ private methods ======================================

void ConversationManager::p_InitSubComponents() {
    user_message_fetcher = std::make_unique<conversation::UserMessageFetcher>(w_sdk_root);
    conv_datasource = std::make_unique<conversation::ConvDatasource>(w_sdk_root);
    receive_conversation = std::make_unique<conversation::ReceiveConversation>(w_sdk_root);
    db_opt = std::make_unique<conversation::DBOpt>(w_sdk_root);
    convert = std::make_unique<conversation::Convert>(w_sdk_root);
    conversation_status_handler = std::make_unique<conversation::ConversationStatusHandler>(w_sdk_root);
}

} // namespace roc::imsdk::core