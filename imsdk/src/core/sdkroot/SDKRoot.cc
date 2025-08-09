// date: 2025-07-09
// author: RuanHuipeng
// description: SDKRoot


#ifndef ROC_IM_SDK_CORE_SDKROOT_SDKROOT_CC
#define ROC_IM_SDK_CORE_SDKROOT_SDKROOT_CC

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "base/network/include/LongConnectionClient.h"
#include "imsdk/src/core/network/connection/SDKConnectionManager.h"
#include "imsdk/src/include/config.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/field.hpp>
#include <memory>

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

asio::awaitable<bool> SDKRoot::init_sdk(const Config config) {
    config_ = config;
 
    // 开启网络IO Context
    // auto wark_work = boost::asio::make_work_guard(net_io_context_);
    // std::thread net_thread([this] { net_io_context_.run(); });
    connection_manager_ = std::make_unique<network::SDKConnectionManager>(sdk_io_context);

    group_manager_ = std::make_unique<core::GroupManager>(weak_from_this());
    message_manager_ = std::make_unique<core::MessageManager>(weak_from_this());
    conversation_manager_ = std::make_unique<core::ConversationManager>(weak_from_this());

    // 初始化数据库
    database_ = new WCDB::Database("/root/project/roc_im_sdk/db-data/" + config_.user_id + "_test.db");

    // 初始化MMKV
    std::string rootDir = "/root/project/roc_im_sdk/db-data";
    MMKV::initializeMMKV(rootDir);
    mmkv_ = MMKV::defaultMMKV();

    // 初始化长连接管理器
    co_await connection_manager_->init_and_connect(weak_from_this());
    
    co_return true;
}

network::SDKConnectionManager* SDKRoot::connection_manager() {
    return connection_manager_.get();
}

const Config& SDKRoot::config() {
    return config_;
}

asio::io_context& SDKRoot::net_io_context() {
    return sdk_io_context;
}

WCDB::Database* SDKRoot::database() {
    return database_;
}

MMKV* SDKRoot::mmkv() {
    return mmkv_;
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