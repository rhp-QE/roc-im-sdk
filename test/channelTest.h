#include "BaseConfig.h"
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <chrono>
#include <iostream>

namespace asio = boost::asio;
using channel_type = asio::experimental::channel<void(boost::system::error_code, int, double)>;

int cnt = 30000;
auto start = std::chrono::high_resolution_clock::now();
// 使用 C++20 协程
asio::awaitable<void> producer(std::shared_ptr<channel_type> ch) {
    for (int i = 0; i < cnt; ++i) {
        co_await ch->async_send(boost::system::error_code{}, i, i + 1, asio::use_awaitable);
    }
    ch->close(); // 发送完毕后关闭通道
}

asio::awaitable<void> consumer(std::shared_ptr<channel_type> ch) {
    try {
        while (true) {
            auto [value, v1] = co_await ch->async_receive(asio::use_awaitable);
            if (value == cnt - 1) {
                auto end = std::chrono::high_resolution_clock::now();
                std::cout << "Function call time: "
              << double(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / 1000000000.0
              << " s per call" << std::endl;
            }
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() == asio::experimental::channel_errc::channel_closed) {
            std::cout <<e.what()<< std::endl;
        } else {
            throw;
        }
    }
}


void testChannel() {
    
    // 使用智能指针延长 channel 生命周期
    auto ch = std::make_shared<channel_type>(net_io_context, 3);

    // 启动生产者和消费者协程

  
    start = std::chrono::high_resolution_clock::now();
    asio::co_spawn(main_io_context, producer(ch), asio::detached);
    asio::co_spawn(main_io_context, consumer(ch), asio::detached);
}