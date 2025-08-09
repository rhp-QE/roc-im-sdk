#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk {

IMSDK::IMSDK() : sdk_root_(std::make_shared<SDKRoot>()) {
}

IMSDK::~IMSDK() {
}

boost::asio::awaitable<bool> IMSDK::init_sdk(const Config config) {
    return sdk_root_->init_sdk(config);
}


} // namespace roc::imsdk