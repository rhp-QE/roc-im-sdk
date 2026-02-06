#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::conversation {

class ReceiveConversation {
public:
    explicit ReceiveConversation(std::weak_ptr<SDKRoot> sdk_root);

    void Start(CTX_T);

    boost::asio::awaitable<void> HandleReceiveConversation(CTX_T, std::vector<std::shared_ptr<network::ConversationData>> conversations);

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation