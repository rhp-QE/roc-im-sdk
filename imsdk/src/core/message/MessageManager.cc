#include "imsdk/src/core/message/MessageManager.h"

#include "im/base/coroutine.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include "imsdk/src/core/message/private/send/SendMessage.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/message/private/fetcher/ConvMessagesFetcher.h"

namespace roc::imsdk::core {

MessageManager::MessageManager(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {
}

MessageManager::~MessageManager() = default;

void MessageManager::all_component_did_load() {
    /// 创建BD 如果必要
    message::DBOpt::create_message_table_if_need(w_sdk_root_);
    /// 开启消息接收处理逻辑
    message::ReceiveMessage::start(w_sdk_root_);
}

/// 保存网络消息
std::vector<std::shared_ptr<model::MessageModel>> MessageManager::save_net_msgs(std::vector<const network::MsgData *> msgs) {
    return message::SaveMessage::save_net_msgs(w_sdk_root_, msgs);
}

/// 设置 sdk 消息
void MessageManager::set_sdk_msg(const core::message::MessageORM *db_msg) {
    message::SaveMessage::set_sdk_msg(w_sdk_root_, db_msg);
}



// =============================  message api implementations  ======================================

void MessageManager::on_message_update(model::OnMessageUpdateCallbackType callback) {
    on_message_update_callback_ = callback;
}

void MessageManager::on_receive_messages(model::OnReceiveMessagesCallbackType callback) {
    on_receive_message_callback_ = callback;
}

boost::asio::awaitable<bool> MessageManager::delete_message(const std::vector<std::string> &msg_ids) {
    // TODO: Implement delete message
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::recall_message(std::string msg_id) {
    // TODO: Implement recall message
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::update_message_sync_ext(std::string msg_id, std::string key, std::string value) {
    // TODO: Implement update message sync ext
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::mark_messages_as_read(const std::vector<std::string> &msg_ids) {
    // 调用 SaveMessage 的静态方法设置消息为已读
    bool result = message::SaveMessage::mark_messages_as_read(w_sdk_root_, msg_ids);
    co_return result;
}

boost::asio::awaitable<std::shared_ptr<model::MessageModel>> MessageManager::message_for_id(std::string msg_id) {
    // TODO: Implement get message by id
    co_return nullptr;
}

boost::asio::awaitable<std::shared_ptr<model::QueryConvMessagesResult>> MessageManager::messages_for_conv_id(std::string conv_id, int64_t cursor, int64_t limit) {
    // TODO: Implement get messages for conversation
    co_return nullptr;
}

boost::asio::awaitable<std::shared_ptr<model::QueryConvMessagesResult>> MessageManager::messages_when_enter_chat(std::string conv_id) {
    co_await message::ConvMessagesFetcher::fetch_conv_message_list(w_sdk_root_, conv_id);
    co_return nullptr;
}

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> MessageManager::send_message(std::vector<model::SendMsgContext> contexts) {
    return message::SendMessage::send_message(w_sdk_root_, contexts);
}

/// 根据 ID 获取 SDK 消息
std::shared_ptr<model::MessageModel> MessageManager::sdk_msg_for_id(std::string msg_id) {
    return message::SaveMessage::sdk_msg_for_id(w_sdk_root_, msg_id);
}

/// ==================================================================================

} // namespace roc::imsdk::core