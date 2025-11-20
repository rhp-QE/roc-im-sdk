#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include <boost/asio/io_context.hpp>

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

boost::asio::awaitable<bool> IMSDK::DeleteMessage(const std::vector<std::string> &msg_ids) {
    return sdk_root_->MessageManager()->DeleteMessage(msg_ids);
}

boost::asio::awaitable<bool> IMSDK::RecallMessage(std::string msg_id) {
    return sdk_root_->MessageManager()->RecallMessage(msg_id);
}

boost::asio::awaitable<bool> IMSDK::UpdateMessageSyncExt(std::string msg_id, std::string key, std::string value) {
    return sdk_root_->MessageManager()->UpdateMessageSyncExt(msg_id, key, value);
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

void IMSDK::OnConvUpdate(model::OnConvUpdateCallbackType callback) {
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

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> IMSDK::CreateConv(std::vector<std::string> member_user_ids, std::string conv_name) {
    return sdk_root_->ConversationManager()->CreateConv(std::move(member_user_ids), std::move(conv_name));
}

// 设置会话置顶
boost::asio::awaitable<bool> IMSDK::SetConvTop(std::string conv_id, bool is_top) {
    return sdk_root_->ConversationManager()->SetConvTop(conv_id, is_top);
}

// 设置会话免打扰
boost::asio::awaitable<bool> IMSDK::SetConvMute(std::string conv_id, bool is_mute) {
    return sdk_root_->ConversationManager()->SetConvMute(conv_id, is_mute);
}

// 设置会话已读
boost::asio::awaitable<bool> IMSDK::SetConvRead(std::string conv_id) {
    return sdk_root_->ConversationManager()->SetConvRead(conv_id);
}

// 删除会话
boost::asio::awaitable<bool> IMSDK::DeleteConv(std::string conv_id) {
    return sdk_root_->ConversationManager()->DeleteConv(conv_id);
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