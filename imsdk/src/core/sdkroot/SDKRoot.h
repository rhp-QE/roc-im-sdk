// date: 2025-07-09
// author: RuanHuipeng
// description: SDKRoot

#ifndef ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H
#define ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H

#include "MMKV/MMKV.h"
#include "WCDB/Database.hpp"
#include "imsdk/base/include/uncopyable.h"
#include "imsdk/src/core/injection/Injection.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/include/config.h"
#include "imsdk/src/include/injection/log/ILogger.h"

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

namespace roc::imsdk::core {
    class GroupManager;
    class MessageManager;
    class ConversationManager;
}

namespace roc::imsdk {

namespace asio = boost::asio;

class SDKRoot : public std::enable_shared_from_this<SDKRoot>,
                public roc::base::uncopyable {

public:
    SDKRoot();
    ~SDKRoot();

    // 设置日志器
    void inject_logger(std::shared_ptr<ILogger> logger);
    std::shared_ptr<ILogger> logger();

    // 初始化sdk
    asio::awaitable<bool> init_sdk(const Config config);

    boost::asio::awaitable<bool> login_out();

    // -------------------------------------------------
    MMKV* mmkv();
    const Config& config();
    WCDB::Database* database();
    asio::io_context& net_io_context();
    asio::io_context& sdk_io_context();
    core::GroupManager* group_manager();
    core::MessageManager* message_manager();
    core::ConversationManager* conversation_manager();
    network::SDKConnectionManager* connection_manager();
    // -------------------------------------------------

private:

    MMKV *mmkv_;
    Config config_;
    WCDB::Database *database_;
    std::shared_ptr<ILogger> logger_;

    std::unique_ptr<core::GroupManager> group_manager_;
    std::unique_ptr<core::MessageManager> message_manager_;
    std::unique_ptr<core::ConversationManager> conversation_manager_;
    std::unique_ptr<network::SDKConnectionManager> connection_manager_;

};

} // namespace roc::im::sdk

#endif // ROC_IM_SDK_CORE_SDKROOT_SDKROOT_H