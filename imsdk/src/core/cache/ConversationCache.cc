#include "imsdk/src/core/cache/ConversationCache.h"

namespace roc::imsdk::cache {

ConversationCache::ConversationCache() {
}

ConversationCache::~ConversationCache() {
}


std::pair<std::shared_ptr<model::ConversationModel>, cache::ConvUpdateReason> ConversationCache::update_and_get_sdk_conv(const db::ConversationORM *db_conv) {
    return std::make_pair(nullptr, cache::ConvUpdateReason::CONVUPDATE_NEW);
}

} // namespace roc::imsdk::cache