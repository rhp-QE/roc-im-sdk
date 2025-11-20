#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::conversation {

class ReceiveConversation {
public:
    static void Start(CONTEXT_T);

    static boost::asio::awaitable<void> HandleReceiveConversation(CONTEXT_T, std::vector<std::shared_ptr<network::ConversationInfo>> conversations);
};

} // namespace roc::imsdk::core::conversation