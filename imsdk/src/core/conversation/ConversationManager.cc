#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"
#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

namespace roc::imsdk::core {

ConversationManager::ConversationManager(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {}

ConversationManager::~ConversationManager() = default;

std::vector<std::shared_ptr<model::ConversationModel>> ConversationManager::save_net_convs(std::vector<const network::ConversationInfo *> convs) {
    std::vector<std::shared_ptr<model::ConversationModel>> sdk_convs;
    
    for (const auto &conv : convs) {
        if (!conv) continue;
        
        // 转换为 db 会话
        auto db_conv = core::conversation::Convert::convert_net_conv_to_db_conv(conv);
        if (!db_conv) continue;
        
        // 转换为 sdk 会话
        auto sdk_conv = core::conversation::Convert::convert_db_conv_to_sdk_conv(db_conv.get());
        if (sdk_conv) {
            sdk_convs.push_back(sdk_conv);
        }
    }
    
    return sdk_convs;
}

void ConversationManager::set_sdk_conv(const core::conversation::ConversationORM *conv) {
    if (!conv) return;
    
    // 转换为 sdk 会话并缓存
    auto sdk_conv = core::conversation::Convert::convert_db_conv_to_sdk_conv(conv);
    if (sdk_conv) {
        // TODO: 实现会话缓存逻辑
        // conv_cache_[conv->conv_id] = sdk_conv;
    }
}

std::shared_ptr<model::ConversationModel> ConversationManager::sdk_conv_for_id(std::string conv_id) {
    // TODO: 实现从缓存获取会话的逻辑
    // return conv_cache_[conv_id];
    return nullptr;
}

int64_t ConversationManager::cursor() {
    return cursor_;
}

void ConversationManager::set_cursor(int64_t cursor) {
    cursor_ = cursor;
}

// =============================  conversation api implementations  ======================================

void ConversationManager::on_conv_update(model::OnConvUpdateCallbackType callback) {
    // TODO: Implement conversation update callback
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConversationManager::conv_for_id(std::string conv_id) {
    // TODO: Implement get conversation by id
    co_return nullptr;
}

boost::asio::awaitable<std::shared_ptr<model::QueryUserConvsResult>> ConversationManager::convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit) {
    // TODO: Implement get conversations for user
    co_return nullptr;
}

boost::asio::awaitable<std::shared_ptr<model::QueryUserConvsResult>> ConversationManager::convs_when_login() {
    // TODO: Implement get first screen conversations when logging in
    co_await conversation::UserMessageFetcher::fetch_user_messages(w_sdk_root_);
    co_return nullptr;
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