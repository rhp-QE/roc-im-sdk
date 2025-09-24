#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/operator/CreateConversation.h"
#include "imsdk/src/core/conversation/private/save/SaveConversation.h"
#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include "imsdk/src/core/common/logger_macro.h"

namespace roc::imsdk::core {

ConversationManager::ConversationManager(std::weak_ptr<SDKRoot> w_sdk_root, boost::asio::io_context::executor_type executor) 
    : w_sdk_root_(w_sdk_root), 
      conv_strand_(boost::asio::make_strand(executor))
{}

ConversationManager::~ConversationManager() = default;

void ConversationManager::all_component_did_load() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root_)

    CONTEXT_NEW_V1

    /// 创建数据库表
    conversation::DBOpt::create_conversation_table_if_need(CONTEXT_V);
}

boost::asio::strand<boost::asio::io_context::executor_type> ConversationManager::conv_strand() {
    return conv_strand_;
}

model::OnConvUpdateCallbackType& ConversationManager::on_conv_update_callback() {
    return on_conv_update_callback_;
}

// =============================  conversation api implementations  ======================================

void ConversationManager::on_conv_update(model::OnConvUpdateCallbackType callback) {
    on_conv_update_callback_ = callback;
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConversationManager::conv_for_id(std::string conv_id) {
    CONTEXT_NEW_V1
    co_return co_await conversation::SaveConversation::sdk_conv_for_id(CONTEXT_V, conv_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit) {
    CONTEXT_NEW_V1
    co_return co_await conversation::SaveConversation::load_convs_from_db(CONTEXT_V, cursor, limit, true);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::convs_when_login() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root_, nullptr)
    CONTEXT_NEW_V1

    /// 触发混链拉取
    asio::co_spawn(sdk_root->net_io_context(), conversation::UserMessageFetcher::fetch_user_messages(CONTEXT_V), asio::detached);

    /// 从DB 中加载会话
    auto convs = co_await conversation::SaveConversation::load_convs_from_db(CONTEXT_V, -1, 100, true);
    co_return convs;
}

boost::asio::awaitable<bool> ConversationManager::set_conv_top(std::string conv_id, bool is_top) {
    // TODO: Implement set conversation as top
    co_return false;
}

boost::asio::awaitable<bool> ConversationManager::set_conv_mute(std::string conv_id, bool is_mute) {
    // TODO: Implement set conversation mute
    co_return false;
}

boost::asio::awaitable<bool> ConversationManager::delete_conv(std::string conv_id) {
    // TODO: Implement delete conversation
    co_return false;
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConversationManager::create_conv(std::vector<std::string> member_user_ids, std::string conv_name) {
    CONTEXT_NEW_V1
    co_return co_await conversation::CreateConversation::create_conv(CONTEXT_V, member_user_ids, conv_name);
}

/// =======================================================================================

} // namespace roc::imsdk::core