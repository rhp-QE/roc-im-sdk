#pragma once

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <cstdint>
namespace roc::imsdk::service {

class ConversationRange {
public:
    ConversationRange(std::weak_ptr<SDKRoot> w_sdk_root);
    ~ConversationRange();

    int64_t conv_cursor();
    void update_conv_cursor(int64_t cursor);

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
    int64_t conv_cursor_ = 0;
    std::mutex mutex_;
};

} // namespace roc::imsdk::service