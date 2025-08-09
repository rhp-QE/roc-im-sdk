#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/include/IMSDK.h"

namespace roc::imsdk::core::message {

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> send_message(W_SDK_ROOT, std::vector<model::SendMsgContext> contexts);

} // namespace roc::imsdk::core::message