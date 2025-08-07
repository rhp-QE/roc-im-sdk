#pragma once


#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
namespace roc::imsdk::service {

class SaveMessage {
public:

    static std::vector<std::shared_ptr<model::MessageModel>> save_net_message(std::weak_ptr<SDKRoot> w_root, const std::vector<const network::MsgData *> &msgs);

};

}