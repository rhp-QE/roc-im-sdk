#include "imsdk/src/core/conversation/private/status/ConversationStatusHandler.h"
#include "core/common/macro.h"
#include "core/sdkroot/SDKRoot.h"
#include <memory>

namespace roc::imsdk::core::conversation {

ConversationStatusHandler::ConversationStatusHandler(std::weak_ptr<SDKRoot> root) :
    w_sdk_root(root)
{}

void ConversationStatusHandler::AllComponentDidLoad() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    sdk_root->ConnectionManager()->AddOnPushMessageCallback([](std::shared_ptr<const network::SdkWSResp> resp) {

    });
}
 
}; // roc::imsdk::core::conversation