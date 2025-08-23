#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/save/SaveConversation.h"
#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

namespace roc::imsdk::core {

ConversationManager::ConversationManager(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {}

ConversationManager::~ConversationManager() = default;

void ConversationManager::all_component_did_load() {
    /// 创建数据库表
    conversation::DBOpt::create_conversation_table_if_need(w_sdk_root_);
}

std::vector<std::shared_ptr<model::ConversationModel>> ConversationManager::save_net_convs(std::vector<std::shared_ptr<network::ConversationInfo>> convs) {
    return conversation::SaveConversation::save_net_convs(w_sdk_root_, convs);
}

std::shared_ptr<model::ConversationModel> ConversationManager::sdk_conv_for_id(std::string conv_id) {
    return conversation::SaveConversation::sdk_conv_for_id(w_sdk_root_, conv_id);
}

int64_t ConversationManager::cursor() {
    return conversation::SaveConversation::get_cursor(w_sdk_root_);
}

void ConversationManager::set_cursor(int64_t cursor) {
    conversation::SaveConversation::set_cursor(w_sdk_root_, cursor);
}

// =============================  conversation api implementations  ======================================

void ConversationManager::on_conv_update(model::OnConvUpdateCallbackType callback) {
    on_conv_update_callback_ = callback;
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConversationManager::conv_for_id(std::string conv_id) {
    co_return conversation::SaveConversation::sdk_conv_for_id(w_sdk_root_, conv_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit) {
    return conversation::SaveConversation::load_convs_from_db(w_sdk_root_, cursor, limit, true);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> ConversationManager::convs_when_login() {
    /// 触发混链拉取
    co_await conversation::UserMessageFetcher::fetch_user_messages(w_sdk_root_);
    /// 从DB 中加载会话
    auto convs = co_await conversation::SaveConversation::load_convs_from_db(w_sdk_root_, -1, 100, true);
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

/// =======================================================================================

} // namespace roc::imsdk::core