#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H


#include <functional>
#include <memory>
#include <string>
#include <im/base/coroutine.h>

namespace roc::net {

class Session;

struct Request : public std::enable_shared_from_this<Request> {

    //-------------------
    enum Method {
        GET,
        POST,
    };
    //-------------------

    friend class Session;

    Request& set_url(std::string url) {
        url_ = url;
        return *this;
    }

    Request& set_method(Method method) {
        method_ = method;
        return *this;
    }

    Request& set_body(std::string body) {
        body_ = body;
        return *this;
    }

    Request& set_callback(std::function<void(std::string data)> callback) {
        callback_ = callback;
        return *this;
    }

    Request& set_content_type(std::string content_type) {
        content_type_ = content_type;
        return *this;
    }

    void request();

    coro::co_async<std::string> co_request();

private:
    std::string url_;
    std::string body_;
    std::string content_type_ = "application/json";
    Request::Method method_;
    std::string host_ = "127.0.0.1";
    std::string port_ = "8080";
    std::function<void(std::string data)> callback_;
};

}


#endif