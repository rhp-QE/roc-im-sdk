//
// SDKConnectionManager.cc
//
// IM SDK 连接管理器实现
//
// author: AI Assistant
// date: 2025-01-xx
//

#include "SDKConnectionManager.h"
#include "imsdk/src/core/network/connection/FrontierMessageJsonSerializer.h"
#include "imsdk/base/include/network/LongConnectionClient.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
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
#include "model/network.h"


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
    request_tracker_ = std::make_unique<RequestTracker>(*(sdk_root->config().net_io_context));
    lc_ = std::make_unique<base::net::LongConnectionClient>(p_GenerateNetConfig(sdk_root.get()), *(sdk_root->config().net_io_context));

    // 观察网络状态变更
    lc_->set_connection_status_callback([=, this](bool connected, const std::string &detail) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
        START_TRACK;

        LOG_INFO("WS", "connected_status: {}, error_info: {}", connected, detail);

        const NetworkStatus status = connected ? NetworkStatus::NETWORK_STATUS_CONNECTED : NetworkStatus::NETWORK_STATUS_DISCONNECTED;
        if (!connected && request_tracker_) {
            // transport 断开时必须唤醒所有等待中的请求，避免业务协程永久挂起。
            request_tracker_->FailAll(roc::error::make_error(2102, "connection disconnected", detail));
        }

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

void SDKConnectionManager::AllComponentDidLoad(CTX_T) {
    // 组件加载完成后的初始化逻辑
}


boost::asio::awaitable<std::expected<std::unique_ptr<FrontierMessage>, roc::error::Error>> SDKConnectionManager::SendRequest(CTX_T, std::unique_ptr<FrontierMessage> req) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(1000, "root is expired", "SDKConnectionManager:send_request")))

    if (!req) {
        co_return std::unexpected(roc::error::make_error(1001, "request is null", "SDKConnectionManager:send_request"));
    }

    req->type                 = FrontierMessageType::Request;
    req->timestamp            = roc::imsdk::core::util::CurrentTimestampMs();
    req->request_id           = next_request_id(sdk_root.get());
    req->metadata["track_id"] = std::to_string(TRACK_ID);

    std::string json_str = FrontierMessageJsonSerializer::ToJsonString(*req);

    std::string request_id_str = req->request_id;
    auto channel = request_tracker_->Track(request_id_str);

    LOG_INFO("WS", "send_request, request_id: {}, service: {}, method: {}", request_id_str, req->service, req->method);

    std::vector<char> buffer(json_str.begin(), json_str.end());
    auto result = co_await lc_->send_data(std::move(buffer));
    if (!result) {
        // 发送入队失败要立刻清理 pending；真实写失败会通过连接状态回调 FailAll。
        request_tracker_->Cancel(request_id_str);
        co_return std::unexpected(roc::error::make_error(2103, "send request failed", result.error().to_string()));
    }

    auto response = co_await request_tracker_->Wait(channel);

    // 切换到 sdk_io_context 执行后续代码
    co_await boost::asio::dispatch(sdk_root->sdk_io_context().get_executor(), boost::asio::use_awaitable);

    if (!response) {
        LOG_INFO("WS", "request_failed, request_id: {}, service: {}, method: {}, error: {}", request_id_str, req->service, req->method, response.error().to_string());
        co_return std::unexpected(response.error());
    }
    co_return std::move(response.value());
}

boost::asio::awaitable<void> SDKConnectionManager::handleDataReceived(boost::beast::flat_buffer data) {
    try {
        CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
        START_TRACK;

        // 防御性拆分输入中可能出现的多个 JSON envelope，核心协议仍是一帧一个 JSON。
        std::string raw_frame = boost::beast::buffers_to_string(data.data());
        size_t start = 0;
        while (start < raw_frame.size()) {
            size_t end = raw_frame.find('\n', start);
            std::string frame = raw_frame.substr(start, end == std::string::npos ? std::string::npos : end - start);
            if (!frame.empty()) {
                co_await handleMessageFrame(std::move(frame));
            }
            if (end == std::string::npos) {
                break;
            }
            start = end + 1;
        }

    } catch (const std::exception &e) {
        auto sdk_root = w_sdk_root.lock();
        uint32_t call_track_id = roc::base::util::generate_uint32_random();
        LOG_INFO("WS", "handle_data_received_exception, error: {}", e.what());
    }
    co_return;
}

boost::asio::awaitable<void> SDKConnectionManager::handleMessageFrame(std::string frame) {
    try {
        CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
        START_TRACK;

        auto msg_result = FrontierMessageJsonSerializer::FromJsonBuffer(frame.data(), frame.size());
        if (!msg_result) {
            LOG_INFO("WS", "Failed to parse JSON message: {}", msg_result.error().to_string());
            co_return;
        }
        std::unique_ptr<FrontierMessage> resp = std::make_unique<FrontierMessage>(std::move(msg_result.value()));
        const std::string &request_id = resp->request_id;

        if (!request_id.empty() && (resp->type == FrontierMessageType::Response || resp->type == FrontierMessageType::Error)) {
            // response/error 只能匹配 pending request；超时后的迟到响应直接丢弃。
            if (request_tracker_ && request_tracker_->Complete(request_id, std::move(resp))) {
                co_return;
            }
            LOG_INFO("WS", "drop_stale_response, request_id: {}", request_id);
            co_return;
        }

        LOG_INFO("WS", "handle_long_connection_push_data, request_id: {}", request_id);

        // push 消息只做分发，不与 RequestTracker 共享状态。
        std::vector<OnPushMesageCallbackType> callbacks_tmp;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            callbacks_tmp = on_push_message_callbacks_;
        } // lock

        std::shared_ptr<const network::FrontierMessage> s_resp = std::move(resp);

        // 转发到 sdk 线程处理，避免在网络线程执行业务回调。
        boost::asio::co_spawn(sdk_root->sdk_io_context(), [=, callbacks_tmp = std::move(callbacks_tmp)]->boost::asio::awaitable<void> {
            for (const auto &callback : callbacks_tmp) {
                if (s_resp) {
                    base::util::safe_invoke_block(callback, s_resp);
                }
            }
            co_return;
        }, boost::asio::detached);

    } catch (const std::exception &e) {
        auto sdk_root = w_sdk_root.lock();
        uint32_t call_track_id = roc::base::util::generate_uint32_random();
        LOG_INFO("WS", "handle_message_frame_exception, error: {}", e.what());
    }
    co_return;
}

// =================================== private ===========================================================

base::net::LongConnectionConfig SDKConnectionManager::p_GenerateNetConfig(roc::imsdk::SDKRoot* root) {
    const auto& sdk_config = root->config();
    const std::string host = sdk_config.app_url.empty() ? "localhost" : sdk_config.app_url;
    const std::string port = sdk_config.app_port.empty() ? "6060" : sdk_config.app_port;
    const std::string token = sdk_config.user_token.empty() ? "uid:" + sdk_config.user_id : sdk_config.user_token;

    roc::base::net::LongConnectionConfig config(host, port);

    config
    .set_heartbeat_interval(5000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("ping")
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000)
    .add_header("User-Agent", "LongConnectionClient/1.0")
    .add_header("Authorization", "Bearer " + token)
    // token 是服务端绑定 userID 的唯一身份来源；user_id 只用于一致性校验和日志。
    .add_query_param("token", token)
    .add_query_param("user_id", sdk_config.user_id)
    .add_query_param("deviceID", sdk_config.user_device_id)
    .add_query_param("platform", std::to_string(sdk_config.platform))
    .add_query_param("clientVersion", "RocSDK-c++/1.0")
    .add_query_param("sdk_type", "roc-imsdk-c++");

    return config;
}


} // namespace roc::imsdk::network
