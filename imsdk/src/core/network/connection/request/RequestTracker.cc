#include "RequestTracker.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <utility>
#include <vector>

namespace roc::imsdk::network {

RequestTracker::RequestTracker(boost::asio::io_context& io_context, std::chrono::milliseconds timeout)
    : io_context_(io_context)
    , timeout_(timeout) {}

std::shared_ptr<RequestTracker::channel_type> RequestTracker::Track(const std::string& request_id) {
    auto channel = std::make_shared<channel_type>(io_context_, 1);
    auto timer = std::make_shared<boost::asio::steady_timer>(io_context_);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (auto iter = pending_.find(request_id); iter != pending_.end()) {
            iter->second.timer->cancel();
            pending_.erase(iter);
        }
        pending_[request_id] = PendingRequest{channel, timer};
    }

    // 每个请求独立 timer，超时事件负责清理 map 并唤醒等待协程。
    StartTimeout(request_id, timer);
    return channel;
}

boost::asio::awaitable<RequestTracker::Response> RequestTracker::Wait(std::shared_ptr<channel_type> channel) {
    if (!channel) {
        co_return std::unexpected(roc::error::make_error(2100, "request channel is null"));
    }
    auto response = co_await channel->async_receive(boost::asio::use_awaitable);
    co_return std::move(response);
}

bool RequestTracker::Complete(const std::string& request_id, std::unique_ptr<FrontierMessage> response) {
    std::shared_ptr<channel_type> channel;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = pending_.find(request_id);
        if (iter == pending_.end()) {
            return false;
        }
        channel = iter->second.channel;
        iter->second.timer->cancel();
        pending_.erase(iter);
    }

    Deliver(channel, Response{std::move(response)});
    return true;
}

bool RequestTracker::Fail(const std::string& request_id, roc::error::Error error) {
    std::shared_ptr<channel_type> channel;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = pending_.find(request_id);
        if (iter == pending_.end()) {
            return false;
        }
        channel = iter->second.channel;
        iter->second.timer->cancel();
        pending_.erase(iter);
    }

    Deliver(channel, Response{std::unexpected(std::move(error))});
    return true;
}

void RequestTracker::FailAll(roc::error::Error error) {
    std::vector<std::shared_ptr<channel_type>> channels;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channels.reserve(pending_.size());
        for (auto& [_, pending] : pending_) {
            pending.timer->cancel();
            channels.push_back(pending.channel);
        }
        pending_.clear();
    }

    // 断线清理必须一次性唤醒所有等待者，避免 SendRequest 永久挂起。
    for (auto& channel : channels) {
        Deliver(channel, Response{std::unexpected(error)});
    }
}

void RequestTracker::Cancel(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = pending_.find(request_id);
    if (iter == pending_.end()) {
        return;
    }
    iter->second.timer->cancel();
    pending_.erase(iter);
}

size_t RequestTracker::PendingCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_.size();
}

void RequestTracker::StartTimeout(const std::string& request_id, std::shared_ptr<boost::asio::steady_timer> timer) {
    timer->expires_after(timeout_);
    boost::asio::co_spawn(io_context_, [this, request_id, timer]() -> boost::asio::awaitable<void> {
        boost::system::error_code ec;
        co_await timer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (!ec) {
            Fail(request_id, roc::error::make_error(2101, "request timeout", request_id));
        }
        co_return;
    }, boost::asio::detached);
}

void RequestTracker::Deliver(std::shared_ptr<channel_type> channel, Response response) {
    boost::asio::co_spawn(io_context_, [channel = std::move(channel), response = std::move(response)]() mutable -> boost::asio::awaitable<void> {
        co_await channel->async_send(boost::system::error_code{}, std::move(response), boost::asio::use_awaitable);
        co_return;
    }, boost::asio::detached);
}

} // namespace roc::imsdk::network
