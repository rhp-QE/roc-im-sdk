#include "imsdk/src/core/message/MessageManager.h"

#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"
#include "imsdk/src/core/message/private/send/SendMessage.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/message/private/cmd/CmdMessageOperator.h"
#include "imsdk/src/core/message/private/fetcher/ConvMessagesFetcher.h"
#include "imsdk/src/core/message/private/convert/Convert.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/use_future.hpp>

namespace roc::imsdk::core {

MessageManager::MessageManager(std::shared_ptr<SDKRoot> sdk_root, boost::asio::io_context::executor_type executor) 
    : w_sdk_root(sdk_root), 
      msg_strand_(boost::asio::make_strand(executor))
{
    p_InitSubComponents();
}

MessageManager::~MessageManager() = default;

void MessageManager::AllComponentDidLoad() {
    START_TRACK;

    /// 创建BD 如果必要
    db_opt->CreateMessageTableIfNeed(CONTEXT_V);
    /// 开启消息接收处理逻辑
    receive_message->Start(CONTEXT_V);
    /// 开启命令消息处理逻辑
    cmd_message_operator->Start(CONTEXT_V);
}

void MessageManager::HandleReceiveMessage(CONTEXT_T, std::vector<std::shared_ptr<network::MsgData>> net_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    boost::asio::co_spawn(sdk_root->sdk_io_context(), receive_message->HandleReceiveMessage(CONTEXT_V, net_msgs), boost::asio::detached);
}

boost::asio::strand<boost::asio::io_context::executor_type> MessageManager::MsgStrand() {
    return msg_strand_;
}

// =============================  message api implementations  ======================================

void MessageManager::OnMessages(model::OnMessagesCallbackType callback) {
    on_messages_callback_ = callback;
}

boost::asio::awaitable<bool> MessageManager::DeleteMessage(const std::vector<std::string> &msg_ids) {
    // TODO: Implement delete message
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::RecallMessage(std::string msg_id) {
    // TODO: Implement recall message
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::UpdateMessageSyncExt(std::string msg_id, std::string key, std::string value) {
    // TODO: Implement update message sync ext
    co_return false;
}

boost::asio::awaitable<bool> MessageManager::MarkMessagesAsRead(const std::vector<std::string> &msg_ids) {
    START_TRACK;
    bool result = save_message->MarkMessagesAsRead(CONTEXT_V, msg_ids);
    co_return result;
}

boost::asio::awaitable<std::shared_ptr<model::MessageModel>> MessageManager::MessageForId(std::string msg_id) {
    START_TRACK;
    co_return co_await save_message->SdkMsgForId(CONTEXT_V, msg_id);
}

// 查询DB
boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessageManager::MessagesForConvId(std::string conv_id, int64_t cursor, int64_t limit) {
    START_TRACK;
    co_return co_await save_message->LoadMessageFromDb(CONTEXT_V, conv_id, cursor, limit, true);
}

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> MessageManager::MessagesWhenEnterChat(std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr)
    START_TRACK;

        boost::asio::co_spawn(sdk_root->sdk_io_context(), [=]() -> boost::asio::awaitable<void> {
        CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

        auto msg_manager = sdk_root->MessageManager();
        /// 加载消息区间
        auto ranges = msg_manager->save_message->LoadMessageRangeFromDb(CONTEXT_V, conv_id);
        msg_manager->msg_range_cache_.insert_or_assign(conv_id, ranges);
        
        std::string range_str;
        for (auto& range : ranges) {
            range_str += "[" + std::to_string(range.first) + ", " + std::to_string(range.second) + "] ";
        }
        LOG_INFO("MsgManager", "【load_message_range_from_db】: {}", range_str)
        

        /// 触发单链拉取
        boost::asio::co_spawn(sdk_root->net_io_context(), msg_manager->conv_messages_fetcher->FetchConvMessageList(CONTEXT_V, conv_id), boost::asio::detached);

    }, boost::asio::detached);

    // 从DB 中加载消息
    co_return co_await save_message->LoadMessageFromDb(CONTEXT_V, conv_id, -1, 100, true);
}

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> MessageManager::SendMessage(model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    START_TRACK;
    return send_message_controller->SendMessage(CONTEXT_V, context, callback);
}

/// ==================================================================================

/// ================================ private methods ======================================

void MessageManager::p_InitSubComponents() {
    save_message = std::make_unique<message::SaveMessage>(w_sdk_root);
    receive_message = std::make_unique<message::ReceiveMessage>(w_sdk_root);
    cmd_message_operator = std::make_unique<message::CmdMessageOperator>(w_sdk_root);
    send_message_controller = std::make_unique<message::SendMessageController>(w_sdk_root);
    db_opt = std::make_unique<message::DBOpt>(w_sdk_root);
    convert = std::make_unique<message::Convert>(w_sdk_root);
    conv_messages_fetcher = std::make_unique<message::ConvMessagesFetcher>(w_sdk_root);
}

} // namespace roc::imsdk::core