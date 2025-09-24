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
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
SaveConversation::save_net_conversations(CONTEXT_T, std::vector<std::shared_ptr<network::ConversationInfo>> convs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    // 转换为 db 会话
    auto db_convs = base::util::transform(convs, [](const std::shared_ptr<network::ConversationInfo> &conv) {
        return core::conversation::Convert::convert_net_conv_to_db_conv(conv.get());
    });

    // 转换为 sdk 会话
    auto sdk_convs_copy = base::util::transform(db_convs, [=](const std::shared_ptr<core::conversation::ConversationORM> &conv) {
        return core::conversation::Convert::convert_db_conv_to_sdk_conv(CONTEXT_V, conv.get());
    });

    auto conv_manager = sdk_root->conversation_manager();

    auto sdk_convs = co_await boost::asio::co_spawn(conv_manager->conv_strand(), [=, sdk_convs_copy = std::move(sdk_convs_copy)]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 保存到数据库
        conversation::DBOpt::insert_conversation(CONTEXT_V, db_convs);

        /// 更新会话缓存
        co_return co_await update_conv_cache(CONTEXT_V, std::move(sdk_convs_copy));

    }, boost::asio::use_awaitable);

    co_return sdk_convs;
}

/// 从db 加载会话
boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
    SaveConversation::load_convs_from_db(CONTEXT_T, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->conversation_manager();

    auto sdk_convs = co_await boost::asio::co_spawn(conv_manager->conv_strand(), [=]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 从DB 中获取会话
        auto sdk_convs_copy = conversation::DBOpt::query_conversations(CONTEXT_V, cursor, limit, forward);

        /// 更新会话缓存
        auto sdk_convs = co_await update_conv_cache(CONTEXT_V, std::move(sdk_convs_copy));

        co_return sdk_convs;
    }, boost::asio::use_awaitable);

    auto load_result = std::make_shared<model::LoadUserConvsResult>();
    load_result->has_more = false;
    load_result->cursor = sdk_convs.empty() ? -1 : sdk_convs.back()->last_message_time();
    load_result->convs = std::move(sdk_convs);
    co_return load_result;
}

/// 根据 ID 获取 SDK 会话
boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> SaveConversation::sdk_conv_for_id(CONTEXT_T, const std::string &conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->conversation_manager();

    if (conv_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取会话
    auto it = conv_manager->conv_cache_.at(conv_id);
    if (it) {
        co_return it.value();
    }

    auto sdk_conv = co_await boost::asio::co_spawn(conv_manager->conv_strand(), [=]() -> boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

        /// 二次检查
        auto result = conv_manager->conv_cache_.at(conv_id);
        if (result) {
            co_return result.value();
        }

        /// 从DB 中获取会话
        auto sdk_conv_copy = conversation::DBOpt::conversation_for_id(CONTEXT_V, conv_id);

        /// 更新缓存
        auto sdk_convs = co_await update_conv_cache(CONTEXT_V, {std::move(sdk_conv_copy)});

        co_return sdk_convs.size() > 0 ? sdk_convs.at(0) : nullptr;

    }, boost::asio::use_awaitable);

    co_return sdk_conv;
}

/// 更新会话缓存
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
SaveConversation::update_conv_cache(CONTEXT_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    if (sdk_convs.empty()) {
        co_return std::vector<std::shared_ptr<model::ConversationModel>>();
    }

    auto conv_manager = sdk_root->conversation_manager();
    auto msg_manager = sdk_root->message_manager();

    std::vector<std::shared_ptr<model::ConversationModel>> cached_sdk_convs;

    for (const auto& conv : sdk_convs) {
        if (conv) {
            std::string last_message_client_id = conv->last_message_client_id();
            auto sdk_msg = co_await msg_manager->message_for_id(last_message_client_id);
            if (sdk_msg) {
                conv->last_message_ = std::move(sdk_msg);
            }

            auto cache_sdk_conv = conv_manager->conv_cache_.at(conv->conversation_id(), std::make_shared<model::ConversationModel>());
            cache_sdk_conv->move_from(std::move(*conv));

            cached_sdk_convs.push_back(cache_sdk_conv);
        }
    }

    co_return cached_sdk_convs;
}



} // namespace roc::imsdk::core::conversation
