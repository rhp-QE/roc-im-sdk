#include "SaveConversation.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"

namespace roc::imsdk::core::conversation {

SaveConversation::SaveConversation(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

/// 保存网络会话
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
SaveConversation::SaveNetConversations(CONTEXT_T, std::vector<std::shared_ptr<network::ConversationInfo>> convs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    auto conv_manager = sdk_root->ConversationManager();

    // 转换为 db 会话
    auto db_convs = base::util::transform(convs, [conv_manager](const std::shared_ptr<network::ConversationInfo> &conv) {
        return conv_manager->convert->ConvertNetConvToDbConv(conv.get());
    });

    // 转换为 sdk 会话
    auto sdk_convs_copy = base::util::transform(db_convs, [conv_manager, w_sdk_root = w_sdk_root, call_track_id](const std::shared_ptr<core::conversation::ConversationORM> &conv) {
        return conv_manager->convert->ConvertDbConvToSdkConv(w_sdk_root, call_track_id, conv.get());
    });

    auto sdk_convs = co_await boost::asio::co_spawn(conv_manager->ConvStrand(), [this, conv_manager, db_convs, sdk_convs_copy = std::move(sdk_convs_copy), w_sdk_root = w_sdk_root, call_track_id]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 保存到数据库
        conv_manager->db_opt->InsertConversation(w_sdk_root, call_track_id, db_convs);

        /// 更新会话缓存
        co_return co_await p_UpdateConvCache(w_sdk_root, call_track_id, std::move(sdk_convs_copy));

    }, boost::asio::use_awaitable);

    co_return sdk_convs;
}

/// 从db 加载会话
boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
    SaveConversation::LoadConvsFromDb(CONTEXT_T, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->ConversationManager();

    auto sdk_convs = co_await boost::asio::co_spawn(conv_manager->ConvStrand(), [this, conv_manager, w_sdk_root = w_sdk_root, call_track_id, cursor, limit, forward]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 从DB 中获取会话
        auto sdk_convs_copy = conv_manager->db_opt->QueryConversations(w_sdk_root, call_track_id, cursor, limit, forward);

        /// 更新会话缓存
        auto sdk_convs = co_await p_UpdateConvCache(w_sdk_root, call_track_id, std::move(sdk_convs_copy));

        co_return sdk_convs;
    }, boost::asio::use_awaitable);

    auto load_result = std::make_shared<model::LoadUserConvsResult>();
    load_result->has_more = false;
    load_result->cursor = sdk_convs.empty() ? -1 : sdk_convs.back()->last_message_time();
    load_result->convs = std::move(sdk_convs);
    co_return load_result;
}

/// 根据 ID 获取 SDK 会话
boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> SaveConversation::SdkConvForId(CONTEXT_T, const std::string &conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->ConversationManager();

    if (conv_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取会话
    auto it = conv_manager->conv_cache_.at(conv_id);
    if (it) {
        co_return it.value();
    }

    auto sdk_conv = co_await boost::asio::co_spawn(conv_manager->ConvStrand(), [this, conv_manager, w_sdk_root = w_sdk_root, call_track_id, conv_id]() -> boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

        /// 二次检查
        auto result = conv_manager->conv_cache_.at(conv_id);
        if (result) {
            co_return result.value();
        }

        /// 从DB 中获取会话
        auto sdk_conv_copy = conv_manager->db_opt->ConversationForId(w_sdk_root, call_track_id, conv_id);

        /// 更新缓存
        auto sdk_convs = co_await p_UpdateConvCache(w_sdk_root, call_track_id, {std::move(sdk_conv_copy)});

        co_return sdk_convs.size() > 0 ? sdk_convs.at(0) : nullptr;

    }, boost::asio::use_awaitable);

    co_return sdk_conv;
}

/// 更新会话缓存
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
SaveConversation::p_UpdateConvCache(CONTEXT_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    if (sdk_convs.empty()) {
        co_return std::vector<std::shared_ptr<model::ConversationModel>>();
    }

    auto conv_manager = sdk_root->ConversationManager();
    auto msg_manager = sdk_root->MessageManager();

    std::vector<std::shared_ptr<model::ConversationModel>> cached_sdk_convs;

    for (const auto& conv : sdk_convs) {
        if (conv) {
            std::string last_message_client_id = conv->last_message_client_id();
            auto sdk_msg = co_await msg_manager->MessageForId(last_message_client_id);
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
