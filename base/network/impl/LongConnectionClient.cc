//
// LongConnectionClient.cc
//
// author: Ruan Huipeng
// date : 2025-03-23
//

#include "base/network/include/LongConnectionClient.h"
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <iostream>
#include <chrono>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <memory>
#include <vector>
#include "base/network/include/Error.h"

namespace asio = boost::asio;
namespace beast = boost::beast;

namespace roc::base::net {

LongConnectionClient::LongConnectionClient(LongConnectionConfig config, boost::asio::io_context& io_context)
    : config_(std::move(config))
    , io_context_(io_context)
    , ws_client_(std::make_shared<WSClient>(config_, io_context_))
    , heartbeat_timer_(std::make_unique<boost::asio::steady_timer>(io_context_))
    , last_heartbeat_time_(std::chrono::steady_clock::now())
    , heartbeat_running_(false)
    , auto_reconnect_running_(false)
    , reconnect_attempts_(0)
    , reconnect_timer_(std::make_unique<boost::asio::steady_timer>(io_context_))
    , running_(false)
    , connected_(false) {
    
    ch = std::make_unique<channel_type>(io_context_.get_executor(), 10);
    std::cout << "LongConnectionClient created with config:" << std::endl;
    std::cout << "  Host: " << config_.get_host() << ":" << config_.get_port() << std::endl;
    std::cout << "  Path: " << config_.get_path() << std::endl;
    std::cout << "  Heartbeat interval: " << config_.get_heartbeat_interval() << "ms" << std::endl;
    std::cout << "  Ping payload: " << config_.get_heartbeat_payload() << std::endl;
    std::cout << "  Auto reconnect: " << (config_.is_auto_reconnect_enabled() ? "enabled" : "disabled") << std::endl;
}

LongConnectionClient::~LongConnectionClient() {
    
    // 停止所有异步操作
    running_ = false;
    connected_ = false;
    
    // 停止心跳
    p_stop_heartbeat_timer();
    
    // 停止重连
    auto_reconnect_running_ = false;
    
    // 断开连接
    if (ws_client_ && ws_client_->is_connected()) {
        try {
            asio::co_spawn(io_context_, [this]() -> asio::awaitable<void> {
                co_await ws_client_->close();
            }, asio::detached);
        } catch (const std::exception& e) {
        }
    }
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> LongConnectionClient::connect() {
    try {
        if (connected_) {
            co_return std::unexpected(roc::error::make_error(2001, "Already connected"));
        }

        if (running_) {
            co_return std::unexpected(roc::error::make_error(2002, "Client is already running"));
        }

        // 重置重连计数
        reconnect_attempts_ = 0;
        
        // 使用 IWSClient 接口连接到 WebSocket 服务器
        auto result = co_await ws_client_->connect();
        if (!result) {
            co_return std::unexpected(roc::error::make_error(2003, "Failed to connect", result.error().to_string()));
        }

        // 设置连接状态
        connected_ = true;
        running_ = true;
        last_heartbeat_time_ = std::chrono::steady_clock::now();
        
        // 通知连接状态变化
        p_notify_connection_status(true, "Connected successfully");
        
        // 启动心跳定时器
        p_start_heartbeat_timer();
        
        // 启动数据接收循环
        p_start_receive_loop();

        // 启动数据发送循环
        p_start_send_loop();
        
        co_return true;
    } catch (const std::exception& e) {
        connected_ = false;
        running_ = false;
        co_return std::unexpected(roc::error::make_error(2004, "Connection failed", e.what()));
    }
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> LongConnectionClient::disconnect() {
    try {
        if (!connected_) {
            co_return std::unexpected(roc::error::make_error(2005, "Not connected"));
        }

        // 停止心跳
        p_stop_heartbeat_timer();
        
        // 停止重连
        auto_reconnect_running_ = false;
        
        // 停止运行
        running_ = false;
        
        // 使用 IWSClient 接口断开 WebSocket 连接
        auto result = co_await ws_client_->close();
        if (!result) {
            // 即使关闭失败也要重置状态
            connected_ = false;
            p_notify_connection_status(false, "Disconnect failed: " + result.error().to_string());
            co_return std::unexpected(roc::error::make_error(2006, "Disconnect failed", result.error().to_string()));
        }

        // 设置连接状态
        connected_ = false;
        
        // 通知连接状态变化
        p_notify_connection_status(false, "Disconnected");
        
        co_return true;
    } catch (const std::exception& e) {
        connected_ = false;
        running_ = false;
        co_return std::unexpected(roc::error::make_error(2007, "Disconnect failed", e.what()));
    }
}

bool LongConnectionClient::is_connected() const {
    return connected_ && ws_client_ && ws_client_->is_connected();
}

boost::asio::awaitable<std::expected<size_t, roc::error::Error>> LongConnectionClient::send_data(const void* data, size_t size) {
    try {
        if (!connected_ || !ws_client_) {
            co_return std::unexpected(roc::error::make_error(2008, "Not connected"));
        }

        if (!data || size == 0) {
            co_return std::unexpected(roc::error::make_error(2009, "Invalid data or size"));
        }

        // 使用 IWSClient 接口发送数据
        // 注意：需要将 const void* 转换为 void*
        auto result = co_await ws_client_->send(const_cast<void*>(data), size);
        if (!result) {
            co_return std::unexpected(roc::error::make_error(2010, "Send failed", result.error().to_string()));
        }

        co_return result.value();
    } catch (const std::exception& e) {
        co_return std::unexpected(roc::error::make_error(2011, "Send failed", e.what()));
    }
}

boost::asio::awaitable<std::expected<size_t, roc::error::Error>> LongConnectionClient::send_data(std::vector<char> buf) {
    size_t size = buf.size();
    co_await ch->async_send(boost::system::error_code{}, std::make_shared<std::vector<char>>(std::move(buf)), boost::asio::use_awaitable);
    co_return size;
}

void LongConnectionClient::p_start_send_loop() {
    asio::co_spawn(io_context_, [this]() -> asio::awaitable<void> {
        co_await p_send_loop();
    }, asio::detached);
}

boost::asio::awaitable<void> LongConnectionClient::p_send_loop() {

    while (running_ && connected_) {
        try {
            auto buf = co_await ch->async_receive(boost::asio::use_awaitable);

            // 使用 IWSClient 接口发送数据
            auto result = co_await ws_client_->send(buf->data(), buf->size());
            if (!result) {
                break;
            }
        } catch (const std::exception& e) {
            break;
        }
    }
}

boost::asio::awaitable<std::expected<size_t, roc::error::Error>> LongConnectionClient::send_message(const std::string& message) {
    return send_data(message.data(), message.size());
}

void LongConnectionClient::set_data_received_callback(DataReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    data_received_callback_ = std::move(callback);
}

void LongConnectionClient::set_connection_status_callback(ConnectionStatusCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    connection_status_callback_ = std::move(callback);
}

void LongConnectionClient::p_start_heartbeat_timer() {
    if (heartbeat_running_) {
        return;
    }
    
    heartbeat_running_ = true;
    asio::co_spawn(io_context_, [this]() -> asio::awaitable<void> {
        co_await p_heartbeat_loop();
    }, asio::detached);
}

void LongConnectionClient::p_stop_heartbeat_timer() {
    heartbeat_running_ = false;
    if (heartbeat_timer_) {
        heartbeat_timer_->cancel();
    }
}

boost::asio::awaitable<void> LongConnectionClient::p_heartbeat_loop() {
    while (heartbeat_running_ && connected_) {
        try {
            // 等待心跳间隔
            heartbeat_timer_->expires_after(std::chrono::milliseconds(config_.get_heartbeat_interval()));
            co_await heartbeat_timer_->async_wait(asio::use_awaitable);
            
            // 发送 ping 帧
            co_await p_send_ping();
        } catch (const std::exception& e) {
            break;
        }
    }
}

boost::asio::awaitable<void> LongConnectionClient::p_send_ping() {
    try {
        if (!connected_ || !ws_client_) {
            co_return;
        }
        
        // 使用 IWSClient 接口发送 ping 帧
        auto result = co_await ws_client_->ping(config_.get_heartbeat_payload());
        std::cout << "LongConnectionClient::p_send_ping: " << result.value() << std::endl;
        if (result) {
            last_heartbeat_time_ = std::chrono::steady_clock::now();
        } else {
            // 心跳失败，可能需要重连
            if (config_.is_auto_reconnect_enabled()) {
                p_start_auto_reconnect();
            }
        }
    } catch (const std::exception& e) {
    }
}

void LongConnectionClient::p_start_receive_loop() {
    asio::co_spawn(io_context_, [this]() -> asio::awaitable<void> {
        co_await p_receive_loop();
    }, asio::detached);
}

boost::asio::awaitable<void> LongConnectionClient::p_receive_loop() {
    std::string err;

    while (running_ && connected_) {
        try {
            // 使用 IWSClient 接口读取数据
            boost::beast::flat_buffer buffer = co_await ws_client_->read();
            p_handle_received_data(std::move(buffer));
        } catch (const std::exception& e) {
            err = e.what();
            break;
        }
    }
    
    // 接收循环结束，可能是连接断开
    if (connected_) {
        connected_ = false;
        p_notify_connection_status(false, err);
        
        // 启动自动重连
        if (config_.is_auto_reconnect_enabled()) {
            p_start_auto_reconnect();
        }
    }
}

void LongConnectionClient::p_handle_received_data(const boost::beast::flat_buffer& buffer) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (data_received_callback_) {
        data_received_callback_(buffer);
    }
}

void LongConnectionClient::p_start_auto_reconnect() {
    if (auto_reconnect_running_) {
        return;
    }
    
    auto_reconnect_running_ = true;
    asio::co_spawn(io_context_, [this]() -> asio::awaitable<void> {
        co_await p_auto_reconnect_loop();
    }, asio::detached);
}

boost::asio::awaitable<void> LongConnectionClient::p_auto_reconnect_loop() {
    while (auto_reconnect_running_ && !connected_ && 
           reconnect_attempts_ < config_.get_max_reconnect_attempts()) {
        
        reconnect_attempts_++;
        
        // 等待退避时间
        if (reconnect_attempts_ > 1) {
            reconnect_timer_->expires_after(std::chrono::milliseconds(config_.get_reconnect_backoff()));
            co_await reconnect_timer_->async_wait(asio::use_awaitable);
        }
        
        // 尝试重连
        auto result = co_await connect();
        if (result) {
            reconnect_attempts_ = 0;
            break;
        } else {
        }
    }
    
    if (!connected_) {
        p_notify_connection_status(false, "Max reconnection attempts reached");
    }
    
    auto_reconnect_running_ = false;
}

void LongConnectionClient::p_notify_connection_status(bool connected, const std::string& reason) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    if (connection_status_callback_) {
        connection_status_callback_(connected, reason);
    }
}

} // namespace roc::base::net 