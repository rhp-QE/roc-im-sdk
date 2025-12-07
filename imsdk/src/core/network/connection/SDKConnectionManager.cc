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


namespace {

// 生成请求唯一 ID（内部工具方法）
static std::atomic<int64_t> request_id_generator{0};
static inline std::string next_request_id(roc::imsdk::SDKRoot *root) {
    return std::to_string(request_id_generator++) + "_" + root->config().user_id + "_" + root->config().user_device_id;
}

}; // namespace


namespace roc::imsdk::network {

SDKConnectionManager::SDKConnectionManager(std::shared_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root)
{}

boost::asio::awaitable<bool> SDKConnectionManager::InitAndConnect(std::shared_ptr<SDKRoot> sdk_root) {

    START_TRACK;
    lc_ = std::make_unique<base::net::LongConnectionClient>(p_GenerateNetConfig(sdk_root.get()), *(sdk_root->config().net_io_context));

    // 观察网络状态变更
    lc_->set_connection_status_callback([=, this](bool connected, const std::string &detail) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
        START_TRACK;

        LOG_INFO("WS", "connected_status: {}, error_info: {}", connected, detail);

        const NetworkStatus status = connected ? NetworkStatus::NETWORK_STATUS_CONNECTED : NetworkStatus::NETWORK_STATUS_DISCONNECTED;

        std::vector<OnConnectionStatusChangeCallbackType> callback_tmp;
        {
            std::lock_guard<std::mutex> lock(this->mutex_);
            callback_tmp = this->network_status_change_callback_;
        }

        boost::asio::co_spawn(sdk_root->sdk_io_context(), [callback_tmp = std::move(callback_tmp), status]->boost::asio::awaitable<void>{
            for(const auto& callback : callback_tmp) {
                base::util::safe_invoke_block(callback, status);
            }
            co_return;
        }, boost::asio::detached);
    });

    // 数据接收回调 
    lc_->set_data_received_callback([=, this](boost::beast::flat_buffer data) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
        boost::asio::co_spawn(*(sdk_root->config().net_io_context), this->handleDataReceived(std::move(data)), asio::detached);
    });

    auto res = co_await lc_->connect();
    
    LOG_INFO("WS","init_and_connected: {}, error_info: {}", res.has_value(), res.has_value() ? "nil" : res.error().to_string())

    co_return res.has_value() ? true : false;
}

/// 网络状态
roc::imsdk::network::NetworkStatus SDKConnectionManager::GetNetworkStatus() {
    return lc_->is_connected() ? imsdk::network::NetworkStatus::NETWORK_STATUS_CONNECTED : imsdk::network::NetworkStatus::NETWORK_STATUS_DISCONNECTED;
}

/// 注入网络状态变更回调
void SDKConnectionManager::OnNetworkStatusChange(std::function<void(roc::imsdk::network::NetworkStatus)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    network_status_change_callback_.push_back(std::move(callback));
}

boost::asio::awaitable<bool> SDKConnectionManager::disconnect() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false)
    START_TRACK;

    auto res = co_await lc_->disconnect();

    LOG_INFO("WS", "【disconnect】: {}, 【error_info】: {}", res.has_value(), res.has_value() ? "" : "disconnect failed")

    co_return res.has_value();
}

void SDKConnectionManager::AddOnPushMessageCallback(OnPushMesageCallbackType callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    on_push_message_callbacks_.push_back(callback);
}

void SDKConnectionManager::AllComponentDidLoad() {
    // 组件加载完成后的初始化逻辑
}


boost::asio::awaitable<std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error>> SDKConnectionManager::SendRequest(network::SdkWSReq *req) {
    std::shared_ptr<SDKRoot> root = w_sdk_root.lock();
    if (!root) {
        co_return std::unexpected(roc::error::make_error(1000, "root is expired", "SDKConnectionManager:send_request"));
    }

    req->set_requestid(next_request_id(root.get()));

    auto channel = std::make_shared<channel_type>(*(root->config().net_io_context), 1);
    std::string request_id_str = req->requestid();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map_[request_id_str] = channel;
    }

    std::vector<char> buffer(req->ByteSizeLong());
    req->SerializeToArray(buffer.data(), req->ByteSizeLong());
    auto result = co_await lc_->send_data(std::move(buffer));

    std::unique_ptr<network::SdkWSResp> resp = co_await channel->async_receive(boost::asio::use_awaitable);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map_.erase(request_id_str);
    }

    // 切换到 sdk_io_context 执行后续代码
    co_await boost::asio::dispatch(root->sdk_io_context().get_executor(), boost::asio::use_awaitable);

    co_return std::move(resp);
}

boost::asio::awaitable<void> SDKConnectionManager::handleDataReceived(boost::beast::flat_buffer data) {
    try {
        CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

        std::unique_ptr<network::SdkWSResp> resp = std::make_unique<network::SdkWSResp>();
        resp->ParseFromArray(data.data().data(), data.size());

        uint32_t call_track_id = resp->trackid();
        const std::string &request_id = resp->requestid();

        std::shared_ptr<channel_type> channel;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto iter = channel_map_.find(request_id);
            if (iter != channel_map_.end()) {
                channel = iter->second;
                channel_map_.erase(iter);
            }
        }

        if (!channel) {
            LOG_INFO("WS", "handle_long_connection_push_data, request_id: {}", request_id);

            /// 直接转发给所有消息者消费， 自己进行数据解析
            std::vector<OnPushMesageCallbackType> callbacks_tmp;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                callbacks_tmp = on_push_message_callbacks_;
            } // lock
            std::shared_ptr<const network::SdkWSResp> s_resp = std::move(resp);
            
            // 转发到 sdk 线程处理 避免卡死主线程
            boost::asio::co_spawn(sdk_root->sdk_io_context(), [callbacks_tmp = std::move(callbacks_tmp), s_resp]->boost::asio::awaitable<void> {
                for (const auto &callback : callbacks_tmp) {
                    base::util::safe_invoke_block(callback, s_resp);
                }
                co_return;
            }, boost::asio::detached);
        } else {
            LOG_INFO("WS", "handle_request_response, request_id: {}, service: {}, method: {}", request_id, resp->service(), resp->method());

            // 唤醒请求携程
            co_await channel->async_send(boost::system::error_code{}, std::move(resp), boost::asio::use_awaitable);
        }

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    co_return;
}

// =================================== private ===========================================================

base::net::LongConnectionConfig SDKConnectionManager::p_GenerateNetConfig(roc::imsdk::SDKRoot* root) {
    roc::base::net::LongConnectionConfig config("localhost", "10010");

    config
    .set_heartbeat_interval(5000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("ping")
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000)
    .add_header("User-Agent", "LongConnectionClient/1.0")
    .add_query_param("sendID", root->config().user_id)
    .add_query_param("sdkType", "RocSDK-c++")
    .add_query_param("sdk_type", "roc-imsdk-c++")
    .add_query_param("user_id", root->config().user_id);

    return config;
}


} // namespace roc::imsdk::network 