#include "imsdk/src/core/cache/ConversationCache.h"

namespace roc::imsdk::cache {

ConversationCache::ConversationCache() {
}

ConversationCache::~ConversationCache() {
}


std::vector<std::shared_ptr<model::ConversationModel>> ConversationCache::update_and_get_sdk_conv(const std::vector<std::shared_ptr<db::ConversationORM>> &db_conv) {
    return std::vector<std::shared_ptr<model::ConversationModel>>();
}

std::shared_ptr<model::ConversationModel> ConversationCache::get_sdk_conv(const std::string& conversation_id) {
    return nullptr;
}

} // namespace roc::imsdk::cache