#include "ReceiveConversation.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/save/SaveConversation.h"

namespace roc::imsdk::core::conversation {

void ReceiveConversation::start(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
}

boost::asio::awaitable<void> ReceiveConversation::handle_receive_conversation(W_SDK_ROOT, std::vector<std::shared_ptr<network::ConversationInfo>> conversations) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto conv_manager = sdk_root->conversation_manager();

    // 保存会话
    auto sdk_convs = co_await conversation::SaveConversation::save_net_conversations(w_sdk_root, std::move(conversations));

    auto on_conversation_result = std::make_shared<model::OnConversationResult>();
    on_conversation_result->updated_convs = sdk_convs;

    // 上抛
    base::util::safe_invoke_block(conv_manager->on_conv_update_callback(), on_conversation_result);
}

} // namespace roc::imsdk::core::conversation