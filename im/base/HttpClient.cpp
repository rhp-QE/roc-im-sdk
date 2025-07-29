#include "HttpClient.h"

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <boost/json.hpp>
#include <im/base/coroutine.h>
#include <BaseConfig.h>

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::beast::net::ip::tcp;


namespace roc::net {

class Session : public std::enable_shared_from_this<Session> {

private:

    tcp::resolver resolver;
    tcp::socket socket;
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::response<http::dynamic_body> res;
    boost::asio::steady_timer timer;
    roc::net::Request reqInfo;

public:
    Session(roc::net::Request request, boost::asio::io_context& ioc) :
        socket(ioc),
        timer(ioc),
        resolver(ioc),
        reqInfo(request) {}

    void start() {
        auto self = shared_from_this(); // 关键：保持对象存活

        // 设置超时（可选）
        timer.expires_after(std::chrono::seconds(5));
        timer.async_wait([self](beast::error_code ec) {
            if (!ec) {
                self->socket.cancel();
            }
        });

        // 异步解析域名（直接使用 IP 时可跳过，但需保持逻辑一致）
        resolver.async_resolve(
            reqInfo.host_, 
            reqInfo.port_,
            [self](const boost::system::error_code& ec, tcp::resolver::results_type results) {
                if (ec) return self->fail(ec, "resolve");
                self->async_connect(results);
            }
        );
    }

    void async_connect(tcp::resolver::results_type results) {
        boost::beast::net::async_connect(
            socket, 
            results,
            [self = shared_from_this()](beast::error_code ec, tcp::endpoint) {
                if (ec) return self->fail(ec, "connect");
                self->async_write_request();
            }
        );
    }

    void async_write_request() {
        req.method((reqInfo.method_ == Request::GET) ? http::verb::get : http::verb::post);
        req.target(reqInfo.url_);
        req.set(http::field::host, reqInfo.host_);
        req.set(http::field::content_type, reqInfo.content_type_);
        req.set(http::field::user_agent, "Boost.Beast");
        req.body() = reqInfo.body_;
        req.prepare_payload();

        http::async_write(socket, req, [self = shared_from_this()](beast::error_code ec, size_t) {
            if (ec) return self->fail(ec, "write");
            self->async_read_response();
        });
    }

    void async_read_response() {
        http::async_read(socket, buffer, res, [self = shared_from_this()](beast::error_code ec, size_t) {
            if (ec) return self->fail(ec, "read");

            std::string str = beast::buffers_to_string(self->res.body().data());

            if (self->reqInfo.callback_) {
                self->reqInfo.callback_(str);
            }
        });
    }

    void fail(beast::error_code ec, const char* what) {
        std::cerr << "[错误] " << what << ": " << ec.message() << "\n";
    }
};


void roc::net::Request::request() {

    auto session = std::make_shared<Session>(*this, net_io_context);

    session->start();
}

coro::co_async<std::string> roc::net::Request::co_request() {
    using namespace roc::coro;

    auto self = shared_from_this();
    auto res = co_await co_awaitable_wapper<std::string>([self](CoroPromise<std::string> promise) {
        self->set_callback([promise](std::string res) mutable{
            promise.set_value(res);
        });
        self->request();
    });
    co_return res;
}

}// namespace::roc::net