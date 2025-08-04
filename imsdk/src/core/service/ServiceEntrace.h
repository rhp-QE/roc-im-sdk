#pragma once

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>

namespace roc::imsdk::service {

class ServiceEntrace {
public:
    ServiceEntrace(std::weak_ptr<SDKRoot> sdk_root);
    ~ServiceEntrace();

    boost::asio::awaitable<void> start_service();

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;


};

}