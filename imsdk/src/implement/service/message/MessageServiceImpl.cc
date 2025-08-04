#include "imsdk/src/implement/service/message/MessageServiceImpl.h"
#include <iostream>

namespace roc::imsdk::service {

MessageServiceImpl::MessageServiceImpl(std::weak_ptr<imsdk::SDKRoot> sdk_root) : sdk_root_(sdk_root) {
    std::cout << "MessageServiceImpl" << std::endl;
    message_send_logic_ = std::make_unique<MessageSendLogic>(sdk_root_);
}

MessageServiceImpl::~MessageServiceImpl() {
    std::cout << "~MessageServiceImpl" << std::endl;
}

void MessageServiceImpl::set_delegate(std::unique_ptr<MessageServiceDelegate> delegate) {
    delegate_ = std::move(delegate);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
MessageServiceImpl::send_message(const std::shared_ptr<model::MessageModel>& message) {
    // 消息入库

    // 消息发送
    auto result = co_await message_send_logic_->send_message({message});
    if (!result) {
        co_return std::unexpected(result.error());
    }
    co_return true;
}

boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
MessageServiceImpl::get_messages(const std::string& message_id) {
    co_return std::vector<std::shared_ptr<model::MessageModel>>();
}

boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
MessageServiceImpl::get_conversation_messages(const std::string& conversation_id, const std::uint64_t& cursor, const int& limit, bool forward) {
    co_return std::vector<std::shared_ptr<model::MessageModel>>();
}


boost::asio::awaitable<std::expected<bool, roc::error::Error>>
MessageServiceImpl::update_message(const std::shared_ptr<model::MessageModel>& message) {
    co_return true;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
MessageServiceImpl::delete_message(const std::shared_ptr<model::MessageModel>& message) {
    co_return true;
}

} // namespace roc::imsdk::service