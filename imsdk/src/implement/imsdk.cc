#include "imsdk/src/include/imsdk.h"    
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/config.h"
#include "imsdk/src/include/service/message/IMessageService.h"
#include "imsdk/src/include/service/conversation/IConversationService.h"

#include <memory>

namespace roc::imsdk {

IMSDK::IMSDK() : sdk_root_(std::make_shared<SDKRoot>()) {
}

IMSDK::~IMSDK() {
}

boost::asio::awaitable<bool> IMSDK::init_sdk(const Config config) {
    return sdk_root_->init_sdk(config);
}

service::IMessageService* IMSDK::msg_service() {
    return sdk_root_->msg_service();
}

service::IConversationService* IMSDK::conv_service() {
    return sdk_root_->conv_service();
}


} // namespace roc::imsdk