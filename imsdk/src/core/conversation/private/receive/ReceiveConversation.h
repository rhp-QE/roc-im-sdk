#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::conversation {

class ReceiveConversation {
public:
    static void start(W_SDK_ROOT);

    static boost::asio::awaitable<void> handle_receive_conversation(W_SDK_ROOT, std::vector<std::shared_ptr<network::ConversationInfo>> conversations);
};

} // namespace roc::imsdk::core::conversation