//
// LongConnectionClient.h
//
// author: Ruan Huipeng
// date : 2025-03-23
//

#ifndef ROC_NET_LONGCONNECTIONCLIENT_H
#define ROC_NET_LONGCONNECTIONCLIENT_H

#include "imsdk/base/include/network/IWSClient.h"
#include "imsdk/base/include/network/WSClientConfig.h"
#include "imsdk/base/include/network/WSClient.h"
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <functional>
#include <memory>
#include <chrono>
#include <atomic>
#include <mutex>
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace roc::base::net {

/**
 * @brief 长连接客户端配置
 * 
 * 扩展了 WebSocket 客户端配置，增加了心跳相关的配置选项
 */
struct LongConnectionConfig : public WSClientConfig {
    // 心跳相关配置
    uint32_t heartbeat_interval_ = 30000; // 心跳间隔，单位毫秒，默认30秒
    uint32_t heartbeat_timeout_ = 10000;  // 心跳超时时间，单位毫秒，默认10秒
    std::string heartbeat_payload_ = "ping"; // ping 帧负载数据
    bool enable_auto_reconnect_ = true;   // 是否启用自动重连
    uint32_t max_reconnect_attempts_ = 5; // 最大重连次数
    uint32_t reconnect_backoff_ms_ = 1000; // 重连退避时间，单位毫秒

public:
    // 继承构造函数
    using WSClientConfig::WSClientConfig;
    
    /**
     * @brief 设置心跳间隔
     * @param interval 心跳间隔（毫秒）
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_heartbeat_interval(uint32_t interval) {
        heartbeat_interval_ = interval;
        return *this;
    }
    
    /**
     * @brief 设置心跳超时时间
     * @param timeout 心跳超时时间（毫秒）
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_heartbeat_timeout(uint32_t timeout) {
        heartbeat_timeout_ = timeout;
        return *this;
    }
    
    /**
     * @brief 设置 ping 帧负载数据
     * @param payload ping 帧的负载数据
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_heartbeat_payload(const std::string& payload) {
        heartbeat_payload_ = payload;
        return *this;
    }
    
    /**
     * @brief 设置是否启用自动重连
     * @param enable 是否启用
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_auto_reconnect(bool enable) {
        enable_auto_reconnect_ = enable;
        return *this;
    }
    
    /**
     * @brief 设置最大重连次数
     * @param attempts 最大重连次数
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_max_reconnect_attempts(uint32_t attempts) {
        max_reconnect_attempts_ = attempts;
        return *this;
    }
    
    /**
     * @brief 设置重连退避时间
     * @param backoff_ms 退避时间（毫秒）
     * @return 返回自身引用，支持链式调用
     */
    LongConnectionConfig& set_reconnect_backoff(uint32_t backoff_ms) {
        reconnect_backoff_ms_ = backoff_ms;
        return *this;
    }

    // 获取方法
    uint32_t get_heartbeat_interval() const { return heartbeat_interval_; }
    uint32_t get_heartbeat_timeout() const { return heartbeat_timeout_; }
    const std::string& get_heartbeat_payload() const { return heartbeat_payload_; }
    bool is_auto_reconnect_enabled() const { return enable_auto_reconnect_; }
    uint32_t get_max_reconnect_attempts() const { return max_reconnect_attempts_; }
    uint32_t get_reconnect_backoff() const { return reconnect_backoff_ms_; }
};

/**
 * @brief 数据接收回调函数类型
 * @param data 接收到的数据
 * @param size 数据大小
 */
using DataReceivedCallback = std::function<void(boost::beast::flat_buffer data)>;

/**
 * @brief 连接状态变化回调函数类型
 * @param connected 连接状态
 * @param reason 状态变化原因
 */
using ConnectionStatusCallback = std::function<void(bool connected, const std::string& reason)>;

/**
 * @brief 长连接客户端类
 * 
 * 在 WebSocket 客户端基础上封装了长连接功能，包括：
 * - 自动心跳检测
 * - 自动重连机制
 * - 数据接收回调
 * - 连接状态监控
 * - 异步数据发送
 */
class LongConnectionClient : public std::enable_shared_from_this<LongConnectionClient> {
public:
    /**
     * @brief 构造函数
     * @param config 长连接配置
     * @param io_context IO上下文
     */
    explicit LongConnectionClient(LongConnectionConfig config, boost::asio::io_context& io_context);
    
    /**
     * @brief 析构函数
     */
    ~LongConnectionClient();

    // ==================== 连接管理 ====================
    
    /**
     * @brief 连接到服务器
     * @return 连接结果
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> connect();
    
    /**
     * @brief 断开连接
     * @return 断开结果
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> disconnect();
    
    /**
     * @brief 检查是否已连接
     * @return 连接状态
     */
    bool is_connected() const;

    // ==================== 数据发送 ====================
    
    /**
     * @brief 异步发送数据
     * @param data 数据指针
     * @param size 数据大小
     * @return 发送结果
     */
    boost::asio::awaitable<std::expected<size_t, roc::error::Error>> send_data(const void* data, size_t size);

    boost::asio::awaitable<std::expected<size_t, roc::error::Error>> send_data(std::vector<char> buf);
    
    /**
     * @brief 异步发送字符串数据
     * @param message 字符串消息
     * @return 发送结果
     */
    boost::asio::awaitable<std::expected<size_t, roc::error::Error>> send_message(const std::string& message);

    // ==================== 回调注册 ====================
    
    /**
     * @brief 注册数据接收回调
     * @param callback 回调函数
     */
    void set_data_received_callback(std::function<void(boost::beast::flat_buffer data)> callback);
    
    /**
     * @brief 注册连接状态变化回调
     * @param callback 回调函数
     */
    void set_connection_status_callback(std::function<void(bool connected, const std::string& reason)> callback);

    // ==================== 配置获取 ====================
    
    /**
     * @brief 获取配置对象
     * @return 配置对象引用
     */
    const LongConnectionConfig& get_config() const { return config_; }
    
private:

    using ws_type = IWSClient<WSClient>;
    
    // ==================== 私有方法 ====================
    
    /**
     * @brief 启动心跳定时器
     */
    void p_start_heartbeat_timer();
    
    /**
     * @brief 停止心跳定时器
     */
    void p_stop_heartbeat_timer();
    
    /**
     * @brief 心跳循环
     */
    boost::asio::awaitable<void> p_heartbeat_loop();
    
    /**
     * @brief 发送 WebSocket ping 帧
     */
    boost::asio::awaitable<void> p_send_ping();
    
    /**
     * @brief 启动数据接收循环
     */
    void p_start_receive_loop();
    
    /**
     * @brief 数据接收循环
     */
    boost::asio::awaitable<void> p_receive_loop();

    /**启动数据发送循环 */
    void p_start_send_loop();

    /**数据发送循环 */
    boost::asio::awaitable<void> p_send_loop();
    
    /**
     * @brief 处理接收到的数据
     * @param buffer 数据缓冲区
     */
    void p_handle_received_data(const boost::beast::flat_buffer& buffer);
    
    /**
     * @brief 启动自动重连
     */
    void p_start_auto_reconnect();
    
    /**
     * @brief 自动重连循环
     */
    boost::asio::awaitable<void> p_auto_reconnect_loop();
    
    /**
     * @brief 通知连接状态变化
     * @param connected 连接状态
     * @param reason 状态变化原因
     */
    void p_notify_connection_status(bool connected, const std::string& reason);

    // ==================== 私有成员变量 ====================
    
    LongConnectionConfig config_; ///< 长连接配置
    boost::asio::io_context& io_context_; ///< IO上下文引用
    std::shared_ptr<ws_type> ws_client_; ///< WebSocket客户端
    std::unique_ptr<boost::asio::steady_timer> reconnect_timer_; ///< 重连定时器
    
    // 心跳相关
    std::unique_ptr<boost::asio::steady_timer> heartbeat_timer_; ///< 心跳定时器
    std::chrono::steady_clock::time_point last_heartbeat_time_; ///< 最后心跳时间
    std::atomic<bool> heartbeat_running_{false}; ///< 心跳运行状态
    
    // 重连相关
    std::atomic<bool> auto_reconnect_running_{false}; ///< 自动重连运行状态
    std::atomic<uint32_t> reconnect_attempts_{0}; ///< 当前重连次数
    
    // 回调函数
    DataReceivedCallback data_received_callback_; ///< 数据接收回调
    ConnectionStatusCallback connection_status_callback_; ///< 连接状态回调
    
    // 线程安全
    mutable std::mutex callback_mutex_; ///< 回调函数互斥锁
    std::atomic<bool> running_{false}; ///< 运行状态
    std::atomic<bool> connected_{false}; ///< 连接状态

    // 数据缓冲区
    using channel_type = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, std::shared_ptr<std::vector<char>>)>;
    std::unique_ptr<channel_type> ch;
};

} // namespace roc::base::net

#endif // ROC_NET_LONGCONNECTIONCLIENT_H

