#pragma once

#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <memory>
#include <string>

namespace roc::imsdk::cache {

class MessageCache {
public:
    MessageCache();
    ~MessageCache();

    std::shared_ptr<model::MessageModel> sdk_message(const std::string& message_id);
    void set_sdk_message(const std::string& message_id, const std::shared_ptr<model::MessageModel> &sdk_msg);

private:
    std::unordered_map<std::string, std::shared_ptr<model::MessageModel>> sdk_message_map_;

};

}