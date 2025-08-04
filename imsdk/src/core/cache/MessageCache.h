#pragma once

#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <memory>
#include <string>

namespace roc::imsdk::cache {

enum MessageUpdateReson {
    MSGUPDATE_NEW,
    MSGUPDATE_DELETE,
    MSGUPDATE_UPDATE,
};

class MessageCache {
public:
    MessageCache();
    ~MessageCache();

    std::shared_ptr<model::MessageModel> get_sdk_message(const std::string& message_id);

    std::pair<std::shared_ptr<model::MessageModel>, MessageUpdateReson> update_and_get_sdk_message(const db::MessageORM *db_msg);

};

}