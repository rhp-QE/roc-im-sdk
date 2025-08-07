#pragma once

#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::service {

class SaveConversation {
public:
    static std::vector<std::shared_ptr<model::ConversationModel>> save_net_conversation(std::weak_ptr<SDKRoot> w_sdk_root, const std::vector<const network::ConversationInfo *> &convs);
};

}