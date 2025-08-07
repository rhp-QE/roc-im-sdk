#include "imsdk/src/core/service/conversation/SaveConversation.h"
#include "imsdk/src/core/db/operator/MessageOperator.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/utils/Utils.h"
#include "imsdk/src/core/cache/ConversationCache.h"

namespace roc::imsdk::service {

std::vector<std::shared_ptr<model::ConversationModel>> SaveConversation::save_net_conversation(std::weak_ptr<SDKRoot> w_sdk_root, const std::vector<const network::ConversationInfo *> &convs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

    std::vector<db::ConversationORM *> db_convs;

    for (auto *conv : convs) {
        auto db_conv = util::convert_net_conv_to_db_conv(conv);
        db_convs.push_back(db_conv.get());
    }

    /// 保存到db
    db::operate::insert_conversation(sdk_root->database(), db_convs);

    /// 更新缓存
    std::vector<std::shared_ptr<model::ConversationModel>> sdk_convs;
    for (auto *conv : db_convs) {
        auto sdk_conv = sdk_root->conversation_cache()->update_and_get_sdk_conv(conv).first;
        sdk_convs.push_back(sdk_conv);
    }

    return sdk_convs;
}

} // namespace roc::imsdk::service
