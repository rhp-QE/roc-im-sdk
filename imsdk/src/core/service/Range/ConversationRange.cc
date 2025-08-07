#include "imsdk/src/core/service/Range/ConversationRange.h"

namespace roc::imsdk::service {

ConversationRange::ConversationRange(std::weak_ptr<SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {
}

ConversationRange::~ConversationRange() = default;

int64_t ConversationRange::conv_cursor() {
    std::lock_guard<std::mutex> lock(mutex_);
    return conv_cursor_;
}

void ConversationRange::update_conv_cursor(int64_t cursor) {
    std::lock_guard<std::mutex> lock(mutex_);
    conv_cursor_ = cursor;
}

} // namespace roc::imsdk::service 