#include "SaveConversation.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"

namespace roc::imsdk::core::conversation {

/// 保存网络会话
std::vector<std::shared_ptr<model::ConversationModel>> SaveConversation::save_net_convs(W_SDK_ROOT, std::vector<std::shared_ptr<network::ConversationInfo>> convs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});
    
    // 转换为 db 会话
    auto db_convs = base::util::transform(convs, [](const std::shared_ptr<network::ConversationInfo> &conv) {
        return core::conversation::Convert::convert_net_conv_to_db_conv(conv.get());
    });

    // 转换为 sdk 会话
    auto sdk_convs = base::util::transform(db_convs, [w_sdk_root](const std::shared_ptr<core::conversation::ConversationORM> &conv) {
        return core::conversation::Convert::convert_db_conv_to_sdk_conv(w_sdk_root, conv.get());
    });

    // 保存到数据库
    conversation::DBOpt::insert_conversation(w_sdk_root, db_convs);
    
    // 更新会话缓存
    update_conv_cache(w_sdk_root, sdk_convs);
    
    return sdk_convs;
}


/// 根据 ID 获取 SDK 会话
std::shared_ptr<model::ConversationModel> SaveConversation::sdk_conv_for_id(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VALUE(conv_manager, nullptr);

    /// 从缓存中获取会话
    auto it = conv_manager->conv_cache_.find(conv_id);
    if (it != conv_manager->conv_cache_.end()) {
        return it->second;
    }

    /// 从DB 中获取会话
    auto db_conv = conversation::DBOpt::query_conversation_by_id(w_sdk_root, conv_id);
    if (db_conv) {
        return db_conv;
    }

    return nullptr;
}

/// 获取会话游标
int64_t SaveConversation::get_cursor(W_SDK_ROOT) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, 0);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VALUE(conv_manager, 0);

    // 通过友元关系访问ConversationManager的私有成员
    return conv_manager->cursor_;
}

/// 设置会话游标
void SaveConversation::set_cursor(W_SDK_ROOT, int64_t cursor) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VOID(conv_manager);

    // 通过友元关系访问ConversationManager的私有成员
    conv_manager->cursor_ = cursor;
}

/// 更新会话缓存
void SaveConversation::update_conv_cache(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> &sdk_convs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    
    if (sdk_convs.empty()) {
        return;
    }

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VOID(conv_manager);

    // 更新会话缓存
    for (const auto& conv : sdk_convs) {
        if (conv) {
            conv_manager->conv_cache_[conv->conversation_id()] = conv;
        }
    }
}

/// 查询会话
boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
    SaveConversation::load_convs_from_db(W_SDK_ROOT, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_CO_RETURN_VALUE(conv_manager, nullptr);

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_CO_RETURN_VALUE(msg_manager, nullptr);

    /// 查询DB
    auto sdk_convs = conversation::DBOpt::query_conversations(w_sdk_root, cursor, limit, forward);

    for (auto &conv : sdk_convs) {
        /// 更新会话缓存
        conv_manager->conv_cache_[conv->conversation_id()] = conv;

        /// 如果最后一条消息为空则填充
        bool need_fill_last_message = !conv->last_message_client_id().empty() && !conv->last_message();
        if (need_fill_last_message) {
            auto sdk_msg = co_await msg_manager->message_for_id(conv->last_message_client_id());
            if (sdk_msg) {
                conv->last_message_ = sdk_msg;
            }
        }
    }

    auto load_result = std::make_shared<model::LoadUserConvsResult>();
    load_result->has_more = false;
    load_result->cursor = sdk_convs.empty() ? -1 : sdk_convs.back()->last_message_time();
    load_result->convs = std::move(sdk_convs);
    co_return load_result;
}

} // namespace roc::imsdk::core::conversation
