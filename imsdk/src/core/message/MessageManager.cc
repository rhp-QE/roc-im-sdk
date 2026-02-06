#include "imsdk/src/core/message/MessageManager.h"

#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/src/core/message/private/send/SendMessage.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/message/private/fetcher/ConvMessagesFetcher.h"
#include "imsdk/src/core/message/private/convert/Convert.h"
#include "imsdk/src/core/message/private/handler/MessageStatusHandler.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>

namespace roc::imsdk::core {

MessageManager::MessageManager(std::shared_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root)
{
    p_InitSubComponents();
}

MessageManager::~MessageManager() = default;

void MessageManager::AllComponentDidLoad() {
    START_TRACK;

    /// 创建BD 如果必要
    db_opt->CreateMessageTableIfNeed(CTX_V);
    /// 开启消息接收处理逻辑
    receive_message->Start(CTX_V);
    /// 注册消息状态处理器
    message_status_handler->AllComponentDidLoad();
}

void MessageManager::HandleReceiveMessage(CTX_T, std::vector<std::shared_ptr<network::MessageData>> net_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    boost::asio::co_spawn(sdk_root->sdk_io_context(), receive_message->HandleReceiveMessage(CTX_V, net_msgs), boost::asio::detached);
}

// =============================  message api implementations  ======================================

void MessageManager::OnMessages(model::OnMessagesCallbackType callback) {
    on_messages_callback_ = callback;
}


model::OnMessagesCallbackType MessageManager::OnMessagesCallback() {
    return on_messages_callback_;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::DeleteMessage(const std::vector<std::string> &msg_ids) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    
    // 批量删除消息
    for (const auto& msg_id : msg_ids) {
        auto result = co_await message_status_handler->Delete(CTX_V, msg_id);
        if (!result) {
            co_return result;
        }
    }
    
    co_return true;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::RecallMessage(std::string msg_id) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await message_status_handler->Recall(CTX_V, msg_id);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::SetMessagePin(std::string msg_id, bool is_pinned) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await message_status_handler->SetPin(CTX_V, msg_id, is_pinned);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::SetMessageSyncExt(std::string msg_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await message_status_handler->SetSyncExt(CTX_V, msg_id, sync_ext);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::SetMessagePropertys(std::string msg_id, const std::vector<int32_t> &propertys) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await message_status_handler->SetPropertys(CTX_V, msg_id, propertys);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> MessageManager::SetMessageLocalExt(std::string msg_id, const std::unordered_map<std::string, std::string> &local_ext) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await message_status_handler->SetLocalExt(CTX_V, msg_id, local_ext);
}

boost::asio::awaitable<bool> MessageManager::MarkMessagesAsRead(const std::vector<std::string> &msg_ids) {
    START_TRACK;
    bool result = message_data_source->MarkMessagesAsRead(CTX_V, msg_ids);
    co_return result;
}

boost::asio::awaitable<std::shared_ptr<model::MessageModel>> MessageManager::MessageForId(std::string msg_id) {
    START_TRACK;
    co_return co_await message_data_source->SdkMsgForId(CTX_V, msg_id);
}

// 查询DB
boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessageManager::MessagesForConvId(std::string conv_id, int64_t cursor, int64_t limit) {
    START_TRACK;
    co_return co_await message_data_source->LoadMessageFromDb(CTX_V, conv_id, cursor, limit, true);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessageManager::MessagesWhenEnterChat(std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    START_TRACK;

     boost::asio::co_spawn(sdk_root->sdk_io_context(), [=, w_sdk_root = w_sdk_root]() -> boost::asio::awaitable<void> {
        CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

        /// 触发单链拉取
        boost::asio::co_spawn(sdk_root->net_io_context(),
         sdk_root->MessageManager()->conv_messages_fetcher->FetchConvMessageList(CTX_V, conv_id),
         boost::asio::detached);

    }, boost::asio::detached);

    // 从DB 中加载消息
    co_return co_await message_data_source->LoadMessageFromDb(CTX_V, conv_id, -1, 100, true);
}

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> MessageManager::SendMessage(model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    START_TRACK;
    return send_message_controller->SendMessage(CTX_V, context, callback);
}

/// ==================================================================================

/// ================================ private methods ======================================

void MessageManager::p_InitSubComponents() {
    message_data_source = std::make_unique<message::MessageDataSource>(w_sdk_root);
    receive_message = std::make_unique<message::ReceiveMessage>(w_sdk_root);
    send_message_controller = std::make_unique<message::SendMessageController>(w_sdk_root);
    db_opt = std::make_unique<message::DBOpt>(w_sdk_root);
    convert = std::make_unique<message::Convert>(w_sdk_root);
    message_status_handler = std::make_unique<message::MessageStatusHandler>(w_sdk_root);
    conv_messages_fetcher = std::make_unique<message::ConvMessagesFetcher>(w_sdk_root);
}

} // namespace roc::imsdk::core