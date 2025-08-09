#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::core::conversation {

boost::asio::awaitable<void> fetch_user_messages(W_SDK_ROOT, std::string user_id);

} // namespace roc::imsdk::core::conversation