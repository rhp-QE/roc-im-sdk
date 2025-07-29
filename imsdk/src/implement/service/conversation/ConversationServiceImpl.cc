#include "ConversationServiceImpl.h"
#include <iostream>

namespace roc::imsdk::service {

ConversationServiceImpl::ConversationServiceImpl(std::weak_ptr<imsdk::SDKRoot> sdk_root) : sdk_root_(sdk_root) {
    std::cout << "ConversationServiceImpl" << std::endl;
}

ConversationServiceImpl::~ConversationServiceImpl() {
    std::cout << "~ConversationServiceImpl" << std::endl;
}

void ConversationServiceImpl::set_delegate(std::unique_ptr<ConversationServiceDelegate> delegate) {
    delegate_ = std::move(delegate);
}

boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::ConversationModel>>, roc::error::Error>>
ConversationServiceImpl::get_conversations() {
    // TODO: Implement
    co_return std::vector<std::shared_ptr<model::ConversationModel>>();
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
ConversationServiceImpl::update_conversation(const std::shared_ptr<model::ConversationModel> &conversation) {
    // TODO: Implement
    co_return true;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
ConversationServiceImpl::delete_conversation(const std::string &conversation_id) {
    // TODO: Implement
    co_return true;
}

} // namespace roc::imsdk::service 