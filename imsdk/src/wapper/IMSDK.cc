#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/group/GroupManager.h"
#include <boost/asio/io_context.hpp>
#include <unordered_map>

namespace roc::imsdk {

IMSDK::IMSDK() : sdk_root_(std::make_shared<SDKRoot>()) {
}

IMSDK::~IMSDK() {
}

/// ========================== injection api ==========================

void IMSDK::InjectLogger(std::shared_ptr<ILogger> logger) {
    sdk_root_->InjectLogger(logger);
}

/// ========================== sdk api ==========================

boost::asio::awaitable<bool> IMSDK::InitSdk(const Config config) {
    return sdk_root_->InitSdk(config);
}

boost::asio::awaitable<bool> IMSDK::run() {
    return sdk_root_->Run();
}

boost::asio::awaitable<bool> IMSDK::LoginOut() {
    return sdk_root_->LoginOut();
}

// =============================  message api  ======================================

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> IMSDK::SendMessage(const model::SendMsgContext &context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    return sdk_root_->MessageManager()->SendMessage(context, callback);
}

void IMSDK::OnMessagee(model::OnMessagesCallbackType callback) {
    sdk_root_->MessageManager()->OnMessages(callback);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::DeleteMessage(const std::vector<std::string> &msg_ids) {
    co_return co_await sdk_root_->MessageManager()->DeleteMessage(msg_ids);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::RecallMessage(std::string msg_id) {
    co_return co_await sdk_root_->MessageManager()->RecallMessage(msg_id);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::UpdateMessageSyncExt(std::string msg_id, std::string key, std::string value) {
    std::unordered_map<std::string, std::string> sync_ext;
    sync_ext[key] = value;
    co_return co_await sdk_root_->MessageManager()->SetMessageSyncExt(msg_id, sync_ext);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetMessagePin(std::string msg_id, bool is_pinned) {
    co_return co_await sdk_root_->MessageManager()->SetMessagePin(msg_id, is_pinned);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetMessagePropertys(std::string msg_id, const std::vector<int32_t> &propertys) {
    co_return co_await sdk_root_->MessageManager()->SetMessagePropertys(msg_id, propertys);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetMessageLocalExt(std::string msg_id, std::string key, std::string value) {
    std::unordered_map<std::string, std::string> local_ext;
    local_ext[key] = value;
    co_return co_await sdk_root_->MessageManager()->SetMessageLocalExt(msg_id, local_ext);
}

boost::asio::awaitable<bool> IMSDK::MarkMessagesAsRead(const std::vector<std::string> &msg_ids) {
    return sdk_root_->MessageManager()->MarkMessagesAsRead(msg_ids);
}

boost::asio::awaitable<std::shared_ptr<model::MessageModel>> IMSDK::MessageForId(std::string msg_id) {
    return sdk_root_->MessageManager()->MessageForId(msg_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> IMSDK::MessagesForConvId(std::string conv_id, int64_t cursor, int64_t limit) {
    return sdk_root_->MessageManager()->MessagesForConvId(conv_id, cursor, limit);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> IMSDK::MessagesWhenEnterChat(std::string conv_id) {
    return sdk_root_->MessageManager()->MessagesWhenEnterChat(conv_id);
}

/// ==================================================================================

// =============================  conversation api  ======================================

void IMSDK::OnConvUpdate(model::OnConversationsCallbackTy callback) {
    sdk_root_->ConversationManager()->OnConvUpdate(callback);
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> IMSDK::ConvForId(std::string conv_id) {
    return sdk_root_->ConversationManager()->ConvForId(conv_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> IMSDK::ConvsForUserId(std::string user_id, int64_t cursor, int64_t limit) {
    return sdk_root_->ConversationManager()->ConvsForUserId(user_id, cursor, limit);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> IMSDK::ConvsWhenLogin() {
    return sdk_root_->ConversationManager()->ConvsWhenLogin();
}

// 创建群聊
boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>> IMSDK::CreateGroup(const model::CreateGroupContext &context) {
    co_return co_await sdk_root_->GroupManager()->CreateGroup(context);
}

// 邀请群成员
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::InviteGroupMembers(const model::InviteGroupMembersContext &context) {
    co_return co_await sdk_root_->GroupManager()->InviteGroupMembers(context);
}

// 设置会话置顶
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetConvTop(std::string conv_id, bool is_top) {
    co_return co_await sdk_root_->ConversationManager()->SetConvTop(conv_id, is_top);
}

// 设置会话免打扰
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetConvMute(std::string conv_id, bool is_mute) {
    co_return co_await sdk_root_->ConversationManager()->SetConvMute(conv_id, is_mute);
}

// 设置会话拉黑
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetConvBlock(std::string conv_id, bool is_block) {
    co_return co_await sdk_root_->ConversationManager()->SetConvBlock(conv_id, is_block);
}

// 设置会话同步扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetConvSyncExt(std::string conv_id, std::string key, std::string value) {
    co_return co_await sdk_root_->ConversationManager()->SetConvSyncExt(conv_id, key, value);
}

// 设置会话本地扩展字段
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::SetConvLocalExt(std::string conv_id, std::string key, std::string value) {
    co_return co_await sdk_root_->ConversationManager()->SetConvLocalExt(conv_id, key, value);
}

// 设置会话已读
boost::asio::awaitable<bool> IMSDK::SetConvRead(std::string conv_id) {
    return sdk_root_->ConversationManager()->SetConvRead(conv_id);
}

// 删除会话
boost::asio::awaitable<std::expected<bool, roc::error::Error>> IMSDK::DeleteConv(std::string conv_id) {
    co_return co_await sdk_root_->ConversationManager()->DeleteConv(conv_id);
}


// =============================  net api  ==============================================

imsdk::network::NetworkStatus IMSDK::GetNetworkStatus() {
    return sdk_root_->ConnectionManager()->GetNetworkStatus();
}

void IMSDK::OnNetworkStatusChange(std::function<void(roc::imsdk::network::NetworkStatus)> callback) {
    sdk_root_->ConnectionManager()->OnNetworkStatusChange(callback);
}

/// =======================================================================================

} // namespace roc::imsdk