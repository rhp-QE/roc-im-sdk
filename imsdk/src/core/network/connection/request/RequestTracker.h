#pragma once

#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/include/model/network.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <expected>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace roc::imsdk::network {

// RequestTracker 是 request_id 的唯一事实源：
// 请求发出后进入 pending；响应、超时、断线任一事件到达都必须清理 pending。
class RequestTracker {
public:
    using Response = std::expected<std::unique_ptr<FrontierMessage>, roc::error::Error>;
    using channel_type = boost::asio::experimental::channel<void(boost::system::error_code, Response)>;

    explicit RequestTracker(boost::asio::io_context& io_context,
                            std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));

    std::shared_ptr<channel_type> Track(const std::string& request_id);

    boost::asio::awaitable<Response> Wait(std::shared_ptr<channel_type> channel);

    bool Complete(const std::string& request_id, std::unique_ptr<FrontierMessage> response);

    bool Fail(const std::string& request_id, roc::error::Error error);

    void FailAll(roc::error::Error error);

    void Cancel(const std::string& request_id);

    size_t PendingCount() const;

private:
    struct PendingRequest {
        std::shared_ptr<channel_type> channel;
        std::shared_ptr<boost::asio::steady_timer> timer;
    };

    void StartTimeout(const std::string& request_id, std::shared_ptr<boost::asio::steady_timer> timer);

    void Deliver(std::shared_ptr<channel_type> channel, Response response);

    boost::asio::io_context& io_context_;
    std::chrono::milliseconds timeout_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, PendingRequest> pending_;
};

} // namespace roc::imsdk::network
