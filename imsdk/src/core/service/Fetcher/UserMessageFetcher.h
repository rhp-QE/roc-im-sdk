///
/// @file   UserMessageFetcher.h
/// @brief  用户消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-22
/// @version 1.0
///


#pragma once

#include "imsdk/src/core/sdkroot/SDKRoot.h"

#include <boost/asio/awaitable.hpp>

namespace roc::imsdk::service {

class UserMessageFetcher : public roc::base::uncopyable {
public:
    UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);
    ~UserMessageFetcher();

    boost::asio::awaitable<bool> fetch_user_messages();

private:
    std::weak_ptr<SDKRoot> sdk_root_;
};

} // namespace roc::imsdk::service