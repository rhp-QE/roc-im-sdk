///
/// @file   ConvMessageFetcher.h
/// @brief  会话消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///


#pragma once

#include "base/Uncopyable.h"
#include <boost/asio/awaitable.hpp>
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::service {


class ConvMessageFetcher : public roc::base::uncopyable {

public:
    ConvMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);
    ~ConvMessageFetcher();

    boost::asio::awaitable<void> fetch_conv_message_list(std::string conv_id);
    boost::asio::awaitable<void> fetch_conv_message_list_for_range(std::string conv_id, std::pair<int64_t, int64_t> range);

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk::service