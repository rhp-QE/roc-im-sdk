// date: 2025-07-09
// author: RuanHuipeng
// description: SDKRoot


#ifndef ROC_IM_SDK_CORE_SDKROOT_SDKROOT_CC
#define ROC_IM_SDK_CORE_SDKROOT_SDKROOT_CC

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/network/LongConnectionClient.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/include/config.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/field.hpp>
#include <memory>

#include "imsdk/src/core/cmd/CmdCenter.h"
#include "imsdk/src/core/group/GroupManager.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"


namespace roc::imsdk {

SDKRoot::SDKRoot() = default;

SDKRoot::~SDKRoot() {
    if (database_) {
        database_->close();
        delete database_;
        database_ = nullptr;
    }
    if (mmkv_) {
        mmkv_->clearAll();
        mmkv_ = nullptr;
    }
}

asio::awaitable<bool> SDKRoot::InitSdk(const Config config) {
    START_TRACK;
    config_ = config;
 
    {
        // 初始化数据库 - 使用相对路径（相对于运行时工作目录）
        // 数据库文件将存储在: <工作目录>/db_data/<user_id>_test.db
        std::string db_path = "./db_data/" + config_.user_id + "_test.db";
        database_ = new WCDB::Database(db_path);

        // 初始化MMKV - 使用相对路径
        // MMKV 文件将存储在: <工作目录>/db_data/
#ifdef _WIN32
        // Windows 上 MMKVPath_t 是 std::wstring
        std::wstring rootDir = L"./db_data";
        MMKV::initializeMMKV(rootDir);
#else
        // Linux/POSIX 上 MMKVPath_t 是 std::string
        std::string rootDir = "./db_data";
        MMKV::initializeMMKV(rootDir);
#endif
        mmkv_ = MMKV::mmkvWithID(config_.user_id);
    }

    auto sdk_root = shared_from_this();
    auto w_sdk_root = weak_from_this();

    {
        cmd_center_ = std::make_unique<core::CmdCenter>(sdk_root);
        group_manager_ = std::make_unique<core::GroupManager>(sdk_root);
        message_manager_ = std::make_unique<core::MessageManager>(sdk_root);
        conversation_manager_ = std::make_unique<core::ConversationManager>(sdk_root);
        connection_manager_ = std::make_unique<network::SDKConnectionManager>(sdk_root);
    }

    {
        cmd_center()->AllComponentDidLoad(CTX_V);
        GroupManager()->AllComponentDidLoad(CTX_V);
        MessageManager()->AllComponentDidLoad(CTX_V);
        ConversationManager()->AllComponentDidLoad(CTX_V);
        ConnectionManager()->AllComponentDidLoad(CTX_V);
    }

    LOG_DEBUG("SDKRoot", "init_sdk {}", "over")
    
    co_return true;
}

/// todo: 运行SDK
boost::asio::awaitable<bool> SDKRoot::Run() {
    return ConnectionManager()->InitAndConnect(shared_from_this());
}

boost::asio::awaitable<bool> SDKRoot::LoginOut() {
    return ConnectionManager()->disconnect();
}

void SDKRoot::InjectLogger(std::shared_ptr<ILogger> logger) {
    logger_ = logger;
}

std::shared_ptr<ILogger> SDKRoot::logger() {
    return logger_;
}

network::SDKConnectionManager* SDKRoot::ConnectionManager() {
    return connection_manager_.get();
}

const Config& SDKRoot::config() {
    return config_;
}

asio::io_context& SDKRoot::net_io_context() {
    return *(config_.net_io_context);
}

asio::io_context& SDKRoot::sdk_io_context() {
    return *(config_.sdk_io_context);
}

asio::io_context& SDKRoot::db_io_context() {
    return *(config_.db_io_context);
}

WCDB::Database* SDKRoot::database() {
    return database_;
}

MMKV* SDKRoot::mmkv() {
    return mmkv_;
}

core::CmdCenter* SDKRoot::cmd_center() {
    return cmd_center_.get();
}

core::GroupManager* SDKRoot::GroupManager() {
    return group_manager_.get();
}

core::MessageManager* SDKRoot::MessageManager() {
    return message_manager_.get();
}

core::ConversationManager* SDKRoot::ConversationManager() {
    return conversation_manager_.get();
}

//--------------- no member private method ----------------------
base::net::LongConnectionConfig generateNetConfig() {
    roc::base::net::LongConnectionConfig config("127.0.0.1", "10010");
    config.set_heartbeat_interval(30000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("ping")
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000)
    .add_header("User-Agent", "LongConnectionClient/1.0")
    .add_query_param("sendID", "RhpUserID")
    .add_query_param("sdkType", "rocSDK-c++");

    return config;
}
//---------------------------------------------------------------

} // namespace roc::imsdk

#endif // ROC_IM_SDK_CORE_SDKROOT_SDKROOT_CC 