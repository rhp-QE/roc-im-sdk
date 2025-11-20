#include "CreateConversation.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::conversation {

CreateConversation::CreateConversation(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

boost::asio::awaitable<std::shared_ptr<model::ConversationModel>>
CreateConversation::CreateConv(CONTEXT_T, std::vector<std::string> member_user_ids, std::string conv_name) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    // TODO: 由业务方在此实现创建会话逻辑
    co_return nullptr;
}

} // namespace roc::imsdk::core::conversation


