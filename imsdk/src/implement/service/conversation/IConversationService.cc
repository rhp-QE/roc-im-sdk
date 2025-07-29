#include "imsdk/src/include/service/conversation/IConversationService.h"
#include "imsdk/src/implement/service/conversation/ConversationServiceImpl.h"

namespace roc::imsdk::service {

IConversationService::IConversationService(std::weak_ptr<imsdk::SDKRoot> sdk_root) 
    : sdk_root_(sdk_root),
      impl_(std::make_unique<ConversationServiceImpl>(sdk_root_)) {}

IConversationService::~IConversationService() = default;

void IConversationService::set_delegate(std::unique_ptr<ConversationServiceDelegate> delegate) {
    impl_->set_delegate(std::move(delegate));
}

boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::ConversationModel>>, roc::error::Error>>
IConversationService::get_conversations() {
    return impl_->get_conversations();
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
IConversationService::update_conversation(const std::shared_ptr<model::ConversationModel> &conversation) {
    return impl_->update_conversation(conversation);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
IConversationService::delete_conversation(const std::string &conversation_id) {
    return impl_->delete_conversation(conversation_id);
}

} // namespace roc::imsdk::service 