// date: 2025-07-09
// author: RuanHuipeng
// description: SDKRoot

#ifndef ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H
#define ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H

#include "MMKV/MMKV.h"
#include "WCDB/Database.hpp"
#include "base/Uncopyable.h"
#include "imsdk/src/core/cache/ConversationCache.h"
#include "imsdk/src/core/cache/MessageCache.h"
#include "imsdk/src/core/injection/Injection.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/include/config.h"

#include <boost/asio/io_context.hpp>
#include <memory>
#include <boost/asio.hpp>
#include <openssl/rsa.h>
#include <boost/asio/detail/concurrency_hint.hpp>
#include "BaseConfig.h"

namespace roc::imsdk::service {
    class IMessageService;
    class IConversationService;
    class ConvMessageFetcher;
    class MessageSendLogic;
    class MessageCacheLogic;
    class UserMessageFetcher;
    class MessageRange;
    class ConversationRange;
}

namespace roc::imsdk {

namespace asio = boost::asio;

class SDKRoot : public std::enable_shared_from_this<SDKRoot>,
                public roc::base::uncopyable {

public:
    SDKRoot();
    ~SDKRoot();

    // 初始化sdk
    asio::awaitable<bool> init_sdk(const Config config);

    // 获取配置信息
    const Config& config();

    // 获取 网络服务
    network::SDKConnectionManager* connection_manager();

    // 获取消息服务
    service::IMessageService* msg_service();

    // 获取会话服务
    service::IConversationService* conv_service();

    // 获取网络IO context
    asio::io_context& net_io_context();

    // 获取数据库
    WCDB::Database* database();

    // 获取MMKV
    MMKV* mmkv();

    // 获取消息缓存
    cache::MessageCache* message_cache();

    // 获取会话缓存
    cache::ConversationCache* conversation_cache();

    // 获取用户消息拉取器
    service::UserMessageFetcher* user_message_fetcher();

    // 获取会话消息拉取器
    service::ConvMessageFetcher* conv_message_fetcher();

    // 获取注入的方法
    injection::Injection* injection();

    // 获取消息区间
    service::MessageRange* message_range();

    // 获取会话区间
    service::ConversationRange* conversation_range();

private:
    std::unique_ptr<network::SDKConnectionManager> connection_manager_;
    std::unique_ptr<service::IMessageService> msg_service_;
    std::unique_ptr<service::IConversationService> conv_service_;
    std::unique_ptr<service::ConvMessageFetcher> conv_message_fetcher_;
    std::unique_ptr<service::MessageSendLogic> send_message_logic_;
    std::unique_ptr<cache::MessageCache> message_cache_;
    std::unique_ptr<cache::ConversationCache> conversation_cache_;
    std::unique_ptr<service::UserMessageFetcher> user_message_fetcher_;
    std::unique_ptr<service::MessageRange> message_range_;
    std::unique_ptr<service::ConversationRange> conversation_range_;
    std::unique_ptr<injection::Injection> injection_;
    Config config_;
    WCDB::Database *database_;
    MMKV *mmkv_;

    asio::io_context net_io_context_;
};

} // namespace roc::im::sdk

#endif // ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H