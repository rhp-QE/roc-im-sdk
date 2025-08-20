#pragma once

#include "imsdk/src/include/IMSDK.h"

namespace roc::imsdk::core {

model::OnMessageResult convert_sdk_msgs_to_receive_msgs_result(std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs);



} // namespace roc::imsdk::core