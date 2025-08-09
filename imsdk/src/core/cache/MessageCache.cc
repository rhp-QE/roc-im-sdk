#include "imsdk/src/core/cache/MessageCache.h"

namespace roc::imsdk::cache {

MessageCache::MessageCache() {
}

MessageCache::~MessageCache() {
}

std::shared_ptr<model::MessageModel> MessageCache::get_sdk_message(const std::string& message_id) {
    return nullptr;
}

std::vector<std::shared_ptr<model::MessageModel>> MessageCache::update_and_get_sdk_message(const std::vector<std::shared_ptr<db::MessageORM>> &db_msg) {
    return std::vector<std::shared_ptr<model::MessageModel>>();
}

} // namespace roc::imsdk::cache    