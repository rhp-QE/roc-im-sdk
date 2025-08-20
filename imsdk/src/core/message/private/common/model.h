#pragma once

#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <memory>
#include <unordered_map>

namespace roc::imsdk::core::message {

struct UpdateMessageResult {
    std::vector<std::shared_ptr<const model::MessageModel>> new_msgs;
    std::vector<std::shared_ptr<const model::MessageModel>> updated_msgs;
    std::unordered_map<std::string, std::shared_ptr<const model::ConversationModel>> convs;
};

} // namespace roc::imsdk::core::message