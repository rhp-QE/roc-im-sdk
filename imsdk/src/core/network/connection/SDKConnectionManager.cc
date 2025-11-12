//
// SDKConnectionManager.cc
//
// IM SDK 连接管理器实现
//
// author: AI Assistant
// date: 2025-01-xx
//

#include "SDKConnectionManager.h"
#include "imsdk/base/include/network/LongConnectionClient.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/json.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/utils/utils.h"

namespace json = boost::json;
namespace asio = boost::asio;

namespace roc::imsdk::network {

//--------------------
base::net::LongConnectionConfig generateNetConfig(roc::imsdk::SDKRoot* root);
//--------------------

SDKConnectionManager::SDKConnectionManager(boost::asio::io_context &io_context) : net_io_context_(io_context) {}

boost::asio::awaitable<void> SDKConnectionManager::init_and_connect(std::weak_ptr<SDKRoot> root) {

    root_ = root;

    CHECK_ROOT_OR_CO_RETURN_VOID(root)

    CONTEXT_NEW_V2

    lc_ = std::make_unique<base::net::LongConnectionClient>(generateNetConfig(root.lock().get()), net_io_context_);

    // 观察网络状态变更
    lc_->set_connection_status_callback([root](bool connected, const std::string &detail) {
        CHECK_ROOT_OR_RETURN_VOID(root)
        CONTEXT_NEW_V2
        LOG_INFO("WS", "connected_status: {}, error_info: {}", connected, detail);

        const NetworkStatus status = connected ? NetworkStatus::NETWORK_STATUS_CONNECTED : NetworkStatus::NETWORK_STATUS_DISCONNECTED;
        base::util::safe_invoke_block(sdk_root->connection_manager()->network_status_change_callback_, status);
    });

    // 
    lc_->set_data_received_callback([wroot = root_](boost::beast::flat_buffer data) {
        std::shared_ptr<SDKRoot> sroot = wroot.lock();
        if (!sroot) {
            return;
        }

        auto conn = sroot->connection_manager();
        boost::asio::co_spawn(conn->net_io_context_, conn->handle_data_received(std::move(data)), asio::detached);
    });

    auto res = co_await lc_->connect();
    
    LOG_INFO("WS","init_and_connected: {}, error_info: {}", res.has_value(), res.has_value() ? "nil" : res.error().to_string())

    co_return;
}

/// 网络状态
roc::imsdk::network::NetworkStatus SDKConnectionManager::get_network_status() {
    return lc_->is_connected() ? imsdk::network::NetworkStatus::NETWORK_STATUS_CONNECTED : imsdk::network::NetworkStatus::NETWORK_STATUS_DISCONNECTED;
}


boost::asio::awaitable<bool> SDKConnectionManager::disconnect() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(root_, false)

    CONTEXT_NEW_V2

    auto res = co_await lc_->disconnect();

    LOG_INFO("WS", "【disconnect】: {}, 【error_info】: {}", res.has_value(), res.has_value() ? "" : "disconnect failed")

    co_return res.has_value();
}

void SDKConnectionManager::add_on_push_message_callback(OnPushMesageCallbackType callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    on_push_message_callbacks.push_back(callback);
}

void SDKConnectionManager::all_component_did_load() {
    // 组件加载完成后的初始化逻辑
}


boost::asio::awaitable<std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error>> SDKConnectionManager::send_request(network::SdkWSReq *req) {
    std::shared_ptr<SDKRoot> root = root_.lock();
    if (!root) {
        co_return std::unexpected(roc::error::make_error(1000, "root is expired", "SDKConnectionManager:send_request"));
    }

    if (req->requestid().empty()) {
        co_return std::unexpected(roc::error::make_error(1000, "requestid is empty", "SDKConnectionManager:send_request"));
    }

    auto channel = std::make_shared<channel_type>(net_io_context_, 1);
    std::string request_id_str = req->requestid();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map[request_id_str] = channel;
    }

    std::vector<char> buffer(req->ByteSizeLong());
    req->SerializeToArray(buffer.data(), req->ByteSizeLong());
    auto result = co_await lc_->send_data(std::move(buffer));

    std::unique_ptr<network::SdkWSResp> resp = co_await channel->async_receive(boost::asio::use_awaitable);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map.erase(request_id_str);
    }

    co_return std::move(resp);
}

boost::asio::awaitable<void> SDKConnectionManager::handle_data_received(boost::beast::flat_buffer data) {
    try {
        CHECK_ROOT_OR_CO_RETURN_VOID(root_)

        std::unique_ptr<network::SdkWSResp> resp = std::make_unique<network::SdkWSResp>();
        resp->ParseFromArray(data.data().data(), data.size());

        uint32_t call_track_id = resp->trackid();
        const std::string &request_id = resp->requestid();

        std::shared_ptr<channel_type> channel;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto iter = channel_map.find(request_id);
            if (iter != channel_map.end()) {
                channel = iter->second;
                channel_map.erase(iter);
            }
        }

        if (!channel) {
            LOG_INFO("WS", "handle_long_connection_push_data, request_id: {}", request_id);

            /// 直接转发给所有消息者消费， 自己进行数据解析
            std::vector<OnPushMesageCallbackType> callbacks;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                callbacks = on_push_message_callbacks;
            } // lock
            std::shared_ptr<network::SdkWSResp> s_resp = std::move(resp);
            for (const auto &callback : callbacks) {
                base::util::safe_invoke_block(callback, s_resp);
            }

        } else {
            LOG_INFO("WS", "handle_request_response, request_id: {}, request_type: {}", request_id, resp->type());

            // 唤醒请求携程
            co_await channel->async_send(boost::system::error_code{}, std::move(resp), boost::asio::use_awaitable);
        }

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    co_return;
}



//--------------- no member private method ----------------------
base::net::LongConnectionConfig generateNetConfig(roc::imsdk::SDKRoot* root) {
    roc::base::net::LongConnectionConfig config("localhost", "10010");
    config.set_heartbeat_interval(5000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("ping")
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000)
    .add_header("User-Agent", "LongConnectionClient/1.0")
    .add_query_param("sendID", root->config().user_id)
    .add_query_param("sdkType", "RocSDK-c++");

    return config;
}
//---------------------------------------------------------------

} // namespace roc::imsdk::network 