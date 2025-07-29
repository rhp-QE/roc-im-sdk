#include "BaseConfig.h"
#include "base/network/include/IWSClient.h"
#include "base/network/include/WSClient.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <boost/asio.hpp>
#include <iostream>
#include <im/base/Utility.h>
#include <memory>
#include <thread>

namespace asio = boost::asio;


// 子协程（切换到线程池的 executor 且不切换回）
asio::awaitable<void> sub_task() {
    auto execter = co_await asio::this_coro::executor;
    co_await asio::post(asio::bind_executor(main_io_context, asio::use_awaitable));
    //co_await asio::post(asio::bind_executor(execter, asio::use_awaitable));
    co_return;
}

// 主协程
asio::awaitable<void> main_coro() {
    std::thread::id thread_id = std::this_thread::get_id();
    co_await sub_task();
    if (thread_id != std::this_thread::get_id()) {
        int a = 100;
    } else {
        int b = 99;
    }
    co_return;
}



boost::asio::awaitable<int> cor1() {
    boost::asio::steady_timer timer(main_io_context, 1);
    co_await timer.async_wait(boost::asio::use_awaitable);
    co_return 100; 
}


boost::asio::awaitable<int> cor2() { 
    int res = co_await cor1();
    co_return res;
}

void boostCoroTest() {
    using ws_type = roc::base::net::WSClient;
    using lc_type = roc::base::net::IWSClient<ws_type>;

    std::shared_ptr<lc_type> lc = std::make_shared<ws_type>(roc::base::net::WSClientConfig{"127.0.0.1",  "8182"}, net_io_context);
    lc->connect();
    boost::asio::co_spawn(net_io_context, main_coro(), boost::asio::detached);
    return;
}
