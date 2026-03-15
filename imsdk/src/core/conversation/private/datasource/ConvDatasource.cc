#include "ConvDatasource.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/convert/convert.h"
#include <boost/json.hpp>

namespace roc::imsdk::core::conversation {

ConvDatasource::ConvDatasource(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root),
      conv_strand_(boost::asio::make_strand(sdk_root.lock()->db_io_context().get_executor())) {
}

boost::asio::strand<boost::asio::io_context::executor_type> ConvDatasource::ConvStrand() {
    return conv_strand_;
}

/// 保存网络会话
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
ConvDatasource::SaveNetConversations(CTX_T, std::vector<std::shared_ptr<network::ConversationData>> convs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    auto conv_manager = sdk_root->ConversationManager();

    // 转换为 db 会话
    auto db_convs = base::util::transform(convs, [=](const std::shared_ptr<network::ConversationData> &conv) {
        return conv_manager->convert->ConvertNetConvToDbConv(CTX_V, conv.get());
    });

    // 转换为 sdk 会话
    auto sdk_convs_copy = base::util::transform(db_convs, [=](const std::shared_ptr<core::conversation::ConversationORM> &conv) {
        return conv_manager->convert->ConvertDbConvToSdkConv(CTX_V, conv.get());
    });

    auto sdk_convs = co_await boost::asio::co_spawn(sdk_root->db_io_context(), [
        =, this, sdk = w_sdk_root,
        sdk_convs_copy = std::move(sdk_convs_copy),
        db_convs = std::move(db_convs)]
        () -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>>
    {
        CHECK_ROOT_OR_CO_RETURN_VALUE(sdk, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 保存到数据库
        conv_manager->db_opt->InsertConversation(CTX_V, db_convs);

        /// 更新会话缓存
        co_return co_await this->p_UpdateConvCache(CTX_V, std::move(sdk_convs_copy));

    }, boost::asio::use_awaitable);

    co_return sdk_convs;
}

/// 从db 加载会话
boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> 
    ConvDatasource::LoadConvsFromDb(CTX_T, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->ConversationManager();

    auto sdk_convs = co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());

        /// 从DB 中获取会话
        auto sdk_convs_copy = conv_manager->db_opt->QueryConversations(CTX_V, cursor, limit, forward);

        /// 更新会话缓存
        auto sdk_convs = co_await this->p_UpdateConvCache(CTX_V, std::move(sdk_convs_copy));

        co_return sdk_convs;
    }, boost::asio::use_awaitable);

    auto load_result = std::make_shared<model::LoadUserConvsResult>();
    load_result->has_more = false;
    load_result->cursor = sdk_convs.empty() ? -1 : sdk_convs.back()->last_message_time();
    load_result->convs = std::move(sdk_convs);
    co_return load_result;
}

/// 根据 ID 获取 SDK 会话
boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConvDatasource::SdkConvForId(CTX_T, const std::string &conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->ConversationManager();

    if (conv_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取会话
    auto it = conv_cache_.at(conv_id);
    if (it) {
        co_return it.value();
    }

    auto sdk_conv = co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

        /// 二次检查
        auto result = conv_cache_.at(conv_id);
        if (result) {
            co_return result.value();
        }

        /// 从DB 中获取会话
        auto sdk_conv_copy = conv_manager->db_opt->ConversationForId(CTX_V, conv_id);

        /// 更新缓存
        auto sdk_convs = co_await this->p_UpdateConvCache(CTX_V, {std::move(sdk_conv_copy)});

        co_return sdk_convs.size() > 0 ? sdk_convs.at(0) : nullptr;

    }, boost::asio::use_awaitable);

    co_return sdk_conv;
}

    /// 根据 ID 获取 SDK 会话
boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> ConvDatasource::SdkConvForIdFromCache(CTX_T, const std::string &conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto conv_manager = sdk_root->ConversationManager();

    if (conv_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取会话
    auto it = conv_cache_.at(conv_id);
    if (it) {
        co_return it.value();
    }

    co_return nullptr;
}

/// 更新会话置顶状态（数据库 + 缓存）
boost::asio::awaitable<bool> ConvDatasource::UpdateConversationTopStatus(CTX_T, const std::string &conv_id, bool is_top) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

    auto conv_manager = sdk_root->ConversationManager();

    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

        // 存储到DB
        bool db_result = conv_manager->db_opt->SetConversationTop(CTX_V, conv_id, is_top);
        if (!db_result) {
            co_return false;
        }

        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            cache_conv.value()->set_top(is_top);
        }

        co_return true;
    }, boost::asio::use_awaitable);
}

/// 更新会话免打扰状态（数据库 + 缓存）
boost::asio::awaitable<bool> ConvDatasource::UpdateConversationMuteStatus(CTX_T, const std::string &conv_id, bool is_mute) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

    auto conv_manager = sdk_root->ConversationManager();

    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

        // 存储到DB
        bool db_result = conv_manager->db_opt->SetConversationMute(CTX_V, conv_id, is_mute);
        if (!db_result) {
            co_return false;
        }

        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            cache_conv.value()->set_mute(is_mute);
        }

        co_return true;
    }, boost::asio::use_awaitable);
}

/// 更新会话拉黑状态（数据库 + 缓存）
boost::asio::awaitable<bool> ConvDatasource::UpdateConversationBlockStatus(CTX_T, const std::string &conv_id, bool is_block) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

    auto conv_manager = sdk_root->ConversationManager();

    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);

        // 存储到DB
        bool db_result = conv_manager->db_opt->SetConversationBlock(CTX_V, conv_id, is_block);
        if (!db_result) {
            co_return false;
        }

        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            cache_conv.value()->set_block(is_block);
        }

        co_return true;
    }, boost::asio::use_awaitable);
}

boost::asio::awaitable<bool> ConvDatasource::UpdateConversationSyncExtStatus(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &sync_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
    auto conv_manager = sdk_root->ConversationManager();
    
    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, &sync_ext, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
        
        // DBOpt 层内部会查询数据库、合并、序列化、写入
        bool db_result = conv_manager->db_opt->SetConversationSyncExt(CTX_V, conv_id, sync_ext);
        if (!db_result) { 
            co_return false; 
        }
        
        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            // 获取当前缓存的 sync_ext，合并传入的 sync_ext
            auto current_sync_ext = cache_conv.value()->sync_ext();
            auto merged_sync_ext = current_sync_ext;
            for (const auto& [key, value] : sync_ext) {
                merged_sync_ext[key] = value;
            }
            cache_conv.value()->set_sync_ext(merged_sync_ext);
        }
        
        co_return true;
    }, boost::asio::use_awaitable);
}

boost::asio::awaitable<bool> ConvDatasource::UpdateConversationLocalExtStatus(CTX_T, const std::string &conv_id, const std::unordered_map<std::string, std::string> &local_ext) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
    auto conv_manager = sdk_root->ConversationManager();
    
    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, &local_ext, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
        
        // DBOpt 层内部会查询数据库、合并、序列化、写入
        bool db_result = conv_manager->db_opt->SetConversationLocalExt(CTX_V, conv_id, local_ext);
        if (!db_result) { 
            co_return false; 
        }
        
        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            // 获取当前缓存的 local_ext，合并传入的 local_ext
            auto current_local_ext = cache_conv.value()->local_ext();
            auto merged_local_ext = current_local_ext;
            for (const auto& [key, value] : local_ext) {
                merged_local_ext[key] = value;
            }
            cache_conv.value()->set_local_ext(merged_local_ext);
        }
        
        co_return true;
    }, boost::asio::use_awaitable);
}

/// 更新会话删除状态（数据库 + 缓存）
boost::asio::awaitable<bool> ConvDatasource::UpdateConversationDeletedStatus(CTX_T, const std::string &conv_id, bool is_deleted) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
    auto conv_manager = sdk_root->ConversationManager();
    
    co_return co_await boost::asio::co_spawn(ConvStrand(), [=, w_sdk_root = w_sdk_root, this]() -> boost::asio::awaitable<bool> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false);
        
        // 存储到DB
        bool db_result = conv_manager->db_opt->DeleteConversation(CTX_V, conv_id);
        if (!db_result) {
            co_return false;
        }
        
        // 如果有则更新缓存
        auto cache_conv = conv_cache_.at(conv_id);
        if (cache_conv) {
            cache_conv.value()->set_deleted(is_deleted);
        }
        
        co_return true;
    }, boost::asio::use_awaitable);
}

/// 更新会话缓存
boost::asio::awaitable<std::vector<std::shared_ptr<model::ConversationModel>>> 
ConvDatasource::p_UpdateConvCache(CTX_T, std::vector<std::shared_ptr<roc::imsdk::model::ConversationModel>> sdk_convs_copy) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::ConversationModel>>());
    
    if (sdk_convs_copy.empty()) {
        co_return std::vector<std::shared_ptr<model::ConversationModel>>();
    }

    auto conv_manager = sdk_root->ConversationManager();
    auto msg_manager = sdk_root->MessageManager();

    std::vector<std::shared_ptr<model::ConversationModel>> cached_sdk_convs;

    for (auto& sdk_conv_copy : sdk_convs_copy) {
        std::string last_message_client_id = sdk_conv_copy->last_message_client_id();
        auto sdk_msg = co_await msg_manager->MessageForId(last_message_client_id);
        if (sdk_msg) {
            sdk_conv_copy->last_message_ = std::move(sdk_msg);
        }

        auto cache_sdk_conv = conv_cache_.at(sdk_conv_copy->conversation_id(), std::move(sdk_conv_copy));
        if (cache_sdk_conv.first) {
            cache_sdk_conv.second->move_from(std::move(*sdk_conv_copy));
        }

        cached_sdk_convs.push_back(cache_sdk_conv.second);
    }

    co_return cached_sdk_convs;
}



} // namespace roc::imsdk::core::conversation

