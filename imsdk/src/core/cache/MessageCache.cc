#include "imsdk/src/core/cache/MessageCache.h"

namespace roc::imsdk::cache {

MessageCache::MessageCache() {
}

MessageCache::~MessageCache() {
}

std::shared_ptr<model::MessageModel> MessageCache::get_sdk_message(const std::string& message_id) {
    return nullptr;
}

std::pair<std::shared_ptr<model::MessageModel>, MessageUpdateReson> MessageCache::update_and_get_sdk_message(const db::MessageORM *db_msg) {
    return std::make_pair(nullptr, MessageUpdateReson::MSGUPDATE_NEW);
}

} // namespace roc::imsdk::cache    