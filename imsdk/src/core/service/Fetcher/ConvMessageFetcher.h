///
/// @file   ConvMessageFetcher.h
/// @brief  会话消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///


#ifndef __IMSDK_MESSAGE_SERVICE_CORE_CONV_MESSAGE_FETCHER_H__
#define __IMSDK_MESSAGE_SERVICE_CORE_CONV_MESSAGE_FETCHER_H__

#include "base/Uncopyable.h"
#include <boost/asio/awaitable.hpp>
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::service {


class ConvMessageFetcher : public roc::base::uncopyable {

public:
    ConvMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);
    ~ConvMessageFetcher();

    boost::asio::awaitable<void> fetch_conv_message_list();

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk::service

#endif