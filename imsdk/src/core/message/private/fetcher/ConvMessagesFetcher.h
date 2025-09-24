#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include <memory>

namespace roc::imsdk::core::message {

class ConvMessagesFetcher {
public:
    /// 获取会话消息列表
    static boost::asio::awaitable<void> fetch_conv_message_list(CONTEXT_T, std::string conv_id);
    
    /// 获取指定区间的会话消息
    static asio::awaitable<void> fetch_conv_message_list_for_range(CONTEXT_T, std::string conv_id, std::pair<int64_t, int64_t> range);
};

} // namespace roc::imsdk::core::message