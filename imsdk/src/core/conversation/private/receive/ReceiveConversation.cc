#include "ReceiveConversation.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/datasource/ConvDatasource.h"

namespace roc::imsdk::core::conversation {

ReceiveConversation::ReceiveConversation(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

void ReceiveConversation::Start(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
}

boost::asio::awaitable<void> ReceiveConversation::HandleReceiveConversation(CTX_T, std::vector<std::shared_ptr<network::ConversationData>> conversations) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto conv_manager = sdk_root->ConversationManager();

    // 保存会话
    auto sdk_convs = co_await conv_manager->conv_datasource->SaveNetConversations(CTX_V, std::move(conversations));

    LOG_INFO("ConvManager", "handleReceiveConversation, sdk_convs: {}", sdk_convs.size());

    // 上抛
    auto on_conversation_result = std::make_shared<model::OnConversationResult>();
    on_conversation_result->fetched_convs = sdk_convs;
    on_conversation_result->insert_convs = sdk_convs;
    base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    co_return;
}

} // namespace roc::imsdk::core::conversation