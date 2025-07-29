#ifndef ROC_IMSDK_IMPLEMENT_SERVICE_MESSAGESERVICE_HPP
#define ROC_IMSDK_IMPLEMENT_SERVICE_MESSAGESERVICE_HPP

#include "imsdk/src/include/service/message/IMessageService.h"
#include "imsdk/src/implement/service/message/MessageServiceImpl.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>

namespace roc::imsdk::service {

    IMessageService::IMessageService(std::weak_ptr<imsdk::SDKRoot> sdk_root) : 
        sdk_root_(sdk_root),
        impl_(std::make_unique<MessageServiceImpl>(sdk_root_)) {}

    IMessageService::~IMessageService() = default;

    void IMessageService::set_delegate(std::unique_ptr<MessageServiceDelegate> delegate) {
        impl_->set_delegate(std::move(delegate));
    }

    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    IMessageService::send_message(std::shared_ptr<model::MessageModel> message) {
        return impl_->send_message(std::move(message));
    }

    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
    IMessageService::get_messages(const std::string& message_id) {
        return impl_->get_messages(message_id);
    }

    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>> 
    IMessageService::get_conversation_messages(const std::string& conversation_id, const std::uint64_t& cursor, const int& limit, bool forward) {
        return impl_->get_conversation_messages(conversation_id, cursor, limit, forward);
    }

    boost::asio::awaitable<std::expected<bool, roc::error::Error>> 
    IMessageService::update_message(const std::shared_ptr<model::MessageModel>& message) {
        return impl_->update_message(message);
    }

    boost::asio::awaitable<std::expected<bool, roc::error::Error>> 
    IMessageService::delete_message(const std::shared_ptr<model::MessageModel>& message) {
        return impl_->delete_message(message);
    }

} // namespace roc::imsdk::service



#endif // ROC_IMSDK_IMPLEMENT_SERVICE_MESSAGESERVICE_HPP 