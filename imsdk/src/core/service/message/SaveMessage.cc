#include "imsdk/src/core/service/message/SaveMessage.h"
#include "imsdk/src/core/db/operator/MessageOperator.h"
#include "imsdk/src/core/utils/Utils.h"
#include "imsdk/src/core/macro.h"

namespace roc::imsdk::service {

std::vector<std::shared_ptr<model::MessageModel>> SaveMessage::save_net_message(std::weak_ptr<SDKRoot> w_sdk_root, const std::vector<const network::MsgData *> &msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());

    std::vector<db::MessageORM *> db_msgs;

    for (auto *msg : msgs) {
        auto db_msg = util::convert_net_msg_to_db_msg(msg);
        db_msgs.push_back(db_msg.get());
    }

    /// 保存到db
    db::operate::insert_message(sdk_root->database(), db_msgs);

    /// 更新缓存
    std::vector<std::shared_ptr<model::MessageModel>> sdk_msgs;
    for (auto *msg : db_msgs) {
        auto sdk_msg = sdk_root->message_cache()->update_and_get_sdk_message(msg).first;
    }

    return sdk_msgs;
}

} // namespace roc::imsdk::service