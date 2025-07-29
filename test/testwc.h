#include "BaseConfig.h"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;
using websocket = beast::websocket::stream<tcp::socket>;

asio::awaitable<void> websocket_client()
{
    try {
        auto executor = co_await asio::this_coro::executor;
        tcp::resolver resolver(executor);
        websocket ws(executor);

        // 解析目标主机和端口
        const std::string host = "echo.websocket.org";
        auto const results = co_await resolver.async_resolve(host, "80", asio::use_awaitable);

        // 连接 TCP
        co_await asio::async_connect(ws.next_layer(), results, asio::use_awaitable);

        // WebSocket 握手
        co_await ws.async_handshake(host, "/", asio::use_awaitable);
        std::cout << "Connected to " << host << std::endl;

        // 发送消息
        std::string message = "Hello, WebSocket!";
        co_await ws.async_write(asio::buffer(message), asio::use_awaitable);
        std::cout << "Sent: " << message << std::endl;

        // 接收响应
        beast::flat_buffer rx_buf;
        co_await ws.async_read(rx_buf, asio::use_awaitable);
        std::cout << "Received: " << beast::buffers_to_string(rx_buf.data()) << std::endl;

        // 优雅关闭连接
        co_await ws.async_close(beast::websocket::close_code::normal, asio::use_awaitable);
        std::cout << "Connection closed" << std::endl;

        beast::error_code ec;
        co_await ws.async_ping(beast::websocket::ping_data{}, 
                               asio::redirect_error(asio::use_awaitable, ec));

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}


void testWC() {
    asio::co_spawn(net_io_context, websocket_client(), asio::detached);
    return;
}