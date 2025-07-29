#ifndef ROC_IMSDK_SERVICE_ICONVERSATIONSERVICE_H
#define ROC_IMSDK_SERVICE_ICONVERSATIONSERVICE_H

#include "base/network/include/Error.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace roc::imsdk {
class SDKRoot;
}

namespace roc::imsdk::service {

// Forward declarations
namespace model {
class ConversationModel;
class ConversationUpdateUnion;
}
class ConversationServiceImpl;

//================================================================
//                 Conversation Service Delegate
//================================================================
struct ConversationServiceDelegate {
    std::function<void(const model::ConversationUpdateUnion &update)> on_conversation_update;
};

//================================================================
//                    Conversation Service
//================================================================
class IConversationService {
public:
    //================================================================
    //                    Lifecycle
    //================================================================
    IConversationService(std::weak_ptr<imsdk::SDKRoot> sdk_root);
    ~IConversationService();

    //================================================================
    //                    Delegate Management
    //================================================================
    void set_delegate(std::unique_ptr<ConversationServiceDelegate> delegate);

    //================================================================
    //                    Conversation Operations
    //================================================================
    // 获取会話列表
    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::ConversationModel>>, roc::error::Error>>
    get_conversations();

    // 更新会話
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    update_conversation(const std::shared_ptr<model::ConversationModel> &conversation);

    // 删除会話
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    delete_conversation(const std::string &conversation_id);

private:
    std::weak_ptr<imsdk::SDKRoot> sdk_root_;
    std::unique_ptr<ConversationServiceImpl> impl_;
};

} // namespace roc::imsdk::service

#endif // ROC_IMSDK_SERVICE_ICONVERSATIONSERVICE_H 