#include "ReceiveConversation.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/save/SaveConversation.h"

namespace roc::imsdk::core::conversation {

ReceiveConversation::ReceiveConversation(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

void ReceiveConversation::Start(CONTEXT_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
}

boost::asio::awaitable<void> ReceiveConversation::HandleReceiveConversation(CONTEXT_T, std::vector<std::shared_ptr<network::ConversationInfo>> conversations) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto conv_manager = sdk_root->ConversationManager();

    // 保存会话
    auto sdk_convs = co_await conv_manager->save_conversation->SaveNetConversations(CONTEXT_V, std::move(conversations));

    LOG_INFO("ConvManager", "handleReceiveConversation, sdk_convs: {}", sdk_convs.size());

    auto on_conversation_result = std::make_shared<model::OnConversationResult>();
    on_conversation_result->updated_convs = sdk_convs;

    // 上抛
    base::util::safe_invoke_block(conv_manager->OnConvUpdateCallback(), on_conversation_result);
}

} // namespace roc::imsdk::core::conversation