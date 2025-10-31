// WSClient.cc
//
// author: Ruan Huipeng
// date : 2025-03-23
//

#include "imsdk/base/include/network/WSClient.h"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;

namespace roc::base::net {

boost::asio::awaitable<std::expected<bool, roc::error::Error>> WSClient::connect_impl() {
    try {
        // 检查配置有效性
        if (config_.get_host().empty() || config_.get_port().empty()) {
            co_return std::unexpected(roc::error::make_error(1001, "Invalid configuration", "Host or port is empty"));
        }

        // 创建解析器并解析主机地址
        auto resolver = boost::asio::ip::tcp::resolver{ io_context_ };
        auto const results = co_await resolver.async_resolve(
            config_.get_host(), 
            config_.get_port(), 
            asio::use_awaitable
        );

        // 设置连接超时时间（使用配置中的重连间隔作为超时时间）
        auto timeout_duration = std::chrono::milliseconds(config_.get_reconnect_interval());
        beast::get_lowest_layer(*ws.get()).expires_after(timeout_duration);

        // 建立 TCP 连接
        auto ep = co_await beast::get_lowest_layer(*ws.get()).async_connect(results, asio::use_awaitable);

        // 关闭 TCP 层的超时，因为 WebSocket 有自己的超时系统
        beast::get_lowest_layer(*ws.get()).expires_never();

        // 设置 WebSocket 建议的超时设置
        ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        // 设置 WebSocket 装饰器，应用配置中的请求头和路径
        ws->set_option(websocket::stream_base::decorator(
            [this](websocket::request_type& req) {
                // 应用配置中的自定义请求头
                for (const auto& header : config_.get_headers()) {
                    req.set(header.first, header.second);
                }
                
                // 设置默认的 User-Agent（如果没有设置的话）
                if (req[http::field::user_agent].empty()) {
                    req.set(http::field::user_agent, 
                           std::string(BOOST_BEAST_VERSION_STRING) + "_websocket-client-coro");
                }
                
                // 设置目标路径（包含查询参数）
                req.target(config_.build_target());
            }
        ));

        // 执行 WebSocket handshake
        // 使用配置中的主机和端口构建 handshake 字符串
        std::string handshake_host = config_.get_host();
        if (config_.get_port() != "80" && config_.get_port() != "443") {
            handshake_host += ":" + config_.get_port();
        }
        
        co_await ws->async_handshake(handshake_host, config_.get_path());

        // 设置连接状态
        connected_ = true;

        co_return true;
    } catch (const std::exception& e) {
        connected_ = false;
        co_return std::unexpected(roc::error::make_error(1002, "Connection failed", e.what()));
    }
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> WSClient::close_impl() {
    try {
        if (!connected_) {
            co_return std::unexpected(roc::error::make_error(1003, "Not connected"));
        }

        // 优雅关闭 WebSocket 连接
        co_await ws->async_close(websocket::close_code::normal, asio::use_awaitable);
        
        // 重置连接状态
        connected_ = false;
        
        co_return true;
    } catch (const std::exception& e) {
        connected_ = false;
        co_return std::unexpected(roc::error::make_error(1004, "Close failed", e.what()));
    }
}

boost::asio::awaitable<std::expected<size_t, roc::error::Error>> WSClient::send_impl(void *data, size_t size) {
    try {
        if (!connected_) {
            co_return std::unexpected(roc::error::make_error(1005, "Not connected"));
        }

        if (!data || size == 0) {
            co_return std::unexpected(roc::error::make_error(1006, "Invalid data or size"));
        }

        // 设置为二进制模式发送
        ws->binary(true);
        
        // 异步发送数据
        auto bytes_sent = co_await ws->async_write(boost::asio::buffer(data, size), asio::use_awaitable);
        
        co_return bytes_sent;
    } catch (const std::exception& e) {
        co_return std::unexpected(roc::error::make_error(1007, "Send failed", e.what()));
    }
}

boost::asio::awaitable<boost::beast::flat_buffer> WSClient::read_impl() {
    try {
        if (!connected_) {
            throw std::runtime_error("Not connected");
        }

        // 创建接收缓冲区
        beast::flat_buffer rx_buf;
        
        // 异步读取数据
        co_await ws->async_read(rx_buf, asio::use_awaitable);
        
        // 注意：Boost.Beast WebSocket 会自动处理 ping/pong 控制帧
        // 当收到 ping 帧时，会自动发送 pong 响应
        // 这里我们只需要处理数据帧
        
        co_return std::move(rx_buf);
    } catch (const std::exception& e) {
        // 读取失败时重置连接状态
        connected_ = false;
        auto res = e.what();
        throw;
    }
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> WSClient::ping_impl(const std::string& payload) {
    try {
        if (!connected_) {
            co_return std::unexpected(roc::error::make_error(1008, "Not connected"));
        }

        // 发送 ping 控制帧
        // 注意：Boost.Beast WebSocket 的 ping/pong 是同步操作
        ws->ping(beast::websocket::ping_data{payload});
        
        co_return true;
    } catch (const std::exception& e) {
        co_return std::unexpected(roc::error::make_error(1009, "Ping failed", e.what()));
    }
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>> WSClient::pong_impl(const std::string& payload) {
    try {
        if (!connected_) {
            co_return std::unexpected(roc::error::make_error(1010, "Not connected"));
        }

        // 发送 pong 控制帧
        // 注意：Boost.Beast WebSocket 的 ping/pong 是同步操作
        ws->pong(beast::websocket::ping_data{payload});
        
        co_return true;
    } catch (const std::exception& e) {
        co_return std::unexpected(roc::error::make_error(1011, "Pong failed", e.what()));
    }
}

bool WSClient::is_connected_impl() {
    return connected_;
}

} // namespace roc::base::net

