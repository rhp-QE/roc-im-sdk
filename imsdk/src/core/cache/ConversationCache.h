#pragma once

#include "imsdk/src/core/db/model/ConversationORM.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include <memory>
#include <string>

namespace roc::imsdk::cache {

enum ConvUpdateReason {
    CONVUPDATE_NEW,
    CONVUPDATE_DELETE,
    CONVUPDATE_UPDATE,
};

class ConversationCache {
public:
    ConversationCache();
    ~ConversationCache();

    std::shared_ptr<model::ConversationModel> get_sdk_conv(const std::string& conversation_id);

    std::pair<std::shared_ptr<model::ConversationModel>, ConvUpdateReason> update_and_get_sdk_conv(const db::ConversationORM *db_conv);

};

}