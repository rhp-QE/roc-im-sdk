#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include <memory>

namespace roc::imsdk::core::message {

boost::asio::awaitable<void> fetch_conv_message_list(W_SDK_ROOT, std::string conv_id);

} // namespace roc::imsdk::core