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

boost::asio::awaitable<bool> IMSDK::init_sdk(const Config config) {
    return sdk_root_->init_sdk(config);
}

boost::asio::awaitable<bool> IMSDK::login_out() {
    return sdk_root_->login_out();
}

// =============================  message api  ======================================

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> IMSDK::send_message(const model::SendMsgContext &context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    return sdk_root_->message_manager()->send_message(context, callback);
}

void IMSDK::on_messagee(model::OnMessagesCallbackType callback) {
    sdk_root_->message_manager()->on_messages(callback);
}

boost::asio::awaitable<bool> IMSDK::delete_message(const std::vector<std::string> &msg_ids) {
    return sdk_root_->message_manager()->delete_message(msg_ids);
}

boost::asio::awaitable<bool> IMSDK::recall_message(std::string msg_id) {
    return sdk_root_->message_manager()->recall_message(msg_id);
}

boost::asio::awaitable<bool> IMSDK::update_message_sync_ext(std::string msg_id, std::string key, std::string value) {
    return sdk_root_->message_manager()->update_message_sync_ext(msg_id, key, value);
}

boost::asio::awaitable<bool> IMSDK::mark_messages_as_read(const std::vector<std::string> &msg_ids) {
    return sdk_root_->message_manager()->mark_messages_as_read(msg_ids);
}

boost::asio::awaitable<std::shared_ptr<model::MessageModel>> IMSDK::message_for_id(std::string msg_id) {
    return sdk_root_->message_manager()->message_for_id(msg_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> IMSDK::messages_for_conv_id(std::string conv_id, int64_t cursor, int64_t limit) {
    return sdk_root_->message_manager()->messages_for_conv_id(conv_id, cursor, limit);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> IMSDK::messages_when_enter_chat(std::string conv_id) {
    return sdk_root_->message_manager()->messages_when_enter_chat(conv_id);
}

/// ==================================================================================

// =============================  conversation api  ======================================

void IMSDK::on_conv_update(model::OnConvUpdateCallbackType callback) {
    sdk_root_->conversation_manager()->on_conv_update(callback);
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> IMSDK::conv_for_id(std::string conv_id) {
    return sdk_root_->conversation_manager()->conv_for_id(conv_id);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> IMSDK::convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit) {
    return sdk_root_->conversation_manager()->convs_for_user_id(user_id, cursor, limit);
}

boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> IMSDK::convs_when_login() {
    return sdk_root_->conversation_manager()->convs_when_login();
}

boost::asio::awaitable<bool> IMSDK::set_conv_top(std::string conv_id, bool is_top) {
    return sdk_root_->conversation_manager()->set_conv_top(conv_id, is_top);
}

boost::asio::awaitable<bool> IMSDK::set_conv_mute(std::string conv_id, bool is_mute) {
    return sdk_root_->conversation_manager()->set_conv_mute(conv_id, is_mute);
}

boost::asio::awaitable<bool> IMSDK::delete_conv(std::string conv_id) {
    return sdk_root_->conversation_manager()->delete_conv(conv_id);
}

/// =======================================================================================

} // namespace roc::imsdk