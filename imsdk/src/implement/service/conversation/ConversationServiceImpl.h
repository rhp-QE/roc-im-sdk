#ifndef ROC_IMSDK_IMPLEMENT_SERVICE_CONVERSATIONSERVICEIMPL_H
#define ROC_IMSDK_IMPLEMENT_SERVICE_CONVERSATIONSERVICEIMPL_H

#include "imsdk/src/include/service/conversation/IConversationService.h"
#include <memory>

namespace roc::imsdk::service {

class ConversationServiceImpl {
public:
    ConversationServiceImpl(std::weak_ptr<imsdk::SDKRoot> sdk_root);
    ~ConversationServiceImpl();

    void set_delegate(std::unique_ptr<ConversationServiceDelegate> delegate);

    // Conversation Operations
    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::ConversationModel>>, roc::error::Error>>
    get_conversations();

    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    update_conversation(const std::shared_ptr<model::ConversationModel> &conversation);
    
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    delete_conversation(const std::string &conversation_id);

private:
    std::weak_ptr<imsdk::SDKRoot> sdk_root_;
    std::unique_ptr<ConversationServiceDelegate> delegate_;
};

} // namespace roc::imsdk::service

#endif // ROC_IMSDK_IMPLEMENT_SERVICE_CONVERSATIONSERVICEIMPL_H 