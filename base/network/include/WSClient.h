//
// WSClient.h
//
// author: Ruan Huipeng
// date : 2025-03-23
// 

#ifndef ROC_NET_WSCLIENT_H
#define ROC_NET_WSCLIENT_H

#include "base/network/include/IWSClient.h"
#include "base/network/include/WSClientConfig.h"
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <string>


namespace roc::base::net {

/**
 * @brief WebSocket 客户端类
 * 
 * 基于 Boost.Beast 和 Boost.Asio 实现的异步 WebSocket 客户端。
 * 支持自动重连、配置管理、异步读写操作等功能。
 * 
 * @tparam WSClient 模板参数，用于 CRTP 模式
 */
class WSClient : public IWSClient<WSClient>, public std::enable_shared_from_this<WSClient> {

    friend IWSClient<WSClient>;

public:
    /**
     * @brief 构造函数
     * 
     * @param config WebSocket 客户端配置对象
     * @param context Boost.Asio IO 上下文，用于异步操作
     */
    explicit WSClient(WSClientConfig config, boost::asio::io_context &context)
        : config_(std::move(config)),
          connected_(false),
          io_context_(context),
          ws(std::make_unique<boost::beast::websocket::stream<boost::beast::tcp_stream>>(context)){}

private:
    // ==================== 核心实现方法 ====================
    
    /**
     * @brief 建立 WebSocket 连接
     * 
     * 异步连接到配置的 WebSocket 服务器，包括：
     * - TCP 连接建立
     * - WebSocket 握手
     * - 设置连接状态
     * 
     * @return 返回连接结果，成功返回 true，失败返回错误信息
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> connect_impl();
    
    /**
     * @brief 关闭 WebSocket 连接
     * 
     * 优雅地关闭 WebSocket 连接，包括：
     * - 发送关闭帧
     * - 关闭底层 TCP 连接
     * - 重置连接状态
     * 
     * @return 返回关闭结果，成功返回 true，失败返回错误信息
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> close_impl();
    
    /**
     * @brief 发送数据
     * 
     * 异步发送二进制数据到 WebSocket 服务器
     * 
     * @param data 要发送的数据指针
     * @param size 数据大小（字节数）
     * @return 返回发送结果，成功返回发送的字节数，失败返回错误信息
     */
    boost::asio::awaitable<std::expected<size_t, roc::error::Error>> send_impl(void *data, size_t size); 
    
    /**
     * @brief 读取数据
     * 
     * 异步从 WebSocket 服务器读取数据
     * 
     * @return 返回读取到的数据缓冲区
     */
    boost::asio::awaitable<boost::beast::flat_buffer> read_impl();
    
    /**
     * @brief 发送 ping 控制帧
     * 
     * @param payload ping 帧的负载数据
     * @return 返回发送结果
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> ping_impl(const std::string& payload);
    
    /**
     * @brief 发送 pong 控制帧
     * 
     * @param payload pong 帧的负载数据
     * @return 返回发送结果
     */
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> pong_impl(const std::string& payload);

    /**
     * @brief 检查连接状态
     * 
     * @return true 如果当前已连接到服务器，false 否则
     */
    bool is_connected_impl();

    // ==================== 私有成员变量 ====================
    
    WSClientConfig config_; ///< WebSocket 客户端配置
    bool connected_; ///< 连接状态标志
    std::unique_ptr<boost::beast::websocket::stream<boost::beast::tcp_stream>> ws; ///< WebSocket 流对象
    boost::asio::io_context &io_context_; ///< IO 上下文引用
};

} // namespace roc::base::net

#endif // ROC_NET_WSCLIENT_H
