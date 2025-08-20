#include "SaveConversation.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
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
    auto sdk_convs = base::util::transform(db_convs, [](const std::shared_ptr<core::conversation::ConversationORM> &conv) {
        return core::conversation::Convert::convert_db_conv_to_sdk_conv(conv.get());
    });

    // 保存到数据库
    conversation::DBOpt::insert_conversation(w_sdk_root, db_convs);
    
    // 更新会话缓存
    update_conv_cache(w_sdk_root, sdk_convs);
    
    return sdk_convs;
}

/// 设置 SDK 会话
void SaveConversation::set_sdk_conv(W_SDK_ROOT, const core::conversation::ConversationORM *conv) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    
    CHECK_POINTER_OR_RETURN_VOID(conv);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VOID(conv_manager);

    // 转换为SDK会话并保存到缓存
    auto sdk_conv = core::conversation::Convert::convert_db_conv_to_sdk_conv(conv);
    if (sdk_conv) {
        // 通过友元关系访问ConversationManager的私有成员
        conv_manager->conv_cache_[conv->conversation_id] = sdk_conv;
    }
}

/// 根据 ID 获取 SDK 会话
std::shared_ptr<model::ConversationModel> SaveConversation::sdk_conv_for_id(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->conversation_manager();
    CHECK_POINTER_OR_RETURN_VALUE(conv_manager, nullptr);

    // 通过友元关系访问ConversationManager的私有成员
    auto it = conv_manager->conv_cache_.find(conv_id);
    if (it != conv_manager->conv_cache_.end()) {
        return it->second;
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

} // namespace roc::imsdk::core::conversation
