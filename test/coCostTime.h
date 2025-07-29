#include "BaseConfig.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>
#include <chrono>
#include <coroutine>
#include <memory>
#include <im/base/coroutine.h>


using namespace roc::coro;


int func() {
    int a = 1;
    for (int i = 0; i < 100; ++i) {
        if (i%2) {
            a += i;
        } else {
            a -= i;
        }

        if (i%3) {
            a += 10;
        } else {
            a -= 100;
        }
    }
    int b = a;
    return b;
    return 0;
}

constexpr int num_iterations = 10000000;

boost::asio::awaitable<std::unique_ptr<int>> coo() {
    auto ptr = std::make_unique<int>(1);
    co_return ptr;
}

boost::asio::awaitable<void> sub_coroutine() {
    func();
    co_return;
}

boost::asio::awaitable<void> par_coroutine() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_iterations; ++i) {
        co_await sub_coroutine();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Coroutine creation time: "
              << double(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / num_iterations
              << " ns per coroutine" << std::endl;
}

boost::asio::awaitable<void> normal_in() {
    return sub_coroutine();
}

int co_cost_time_test() {

    // 测试普通函数调用
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_iterations; ++i) {
        []() {
            func();
        }(); // 空的 lambda 表达式，模拟函数调用
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Function call time: "
              << double(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / num_iterations
              << " ns per call" << std::endl;

    boost::asio::co_spawn(main_io_context, par_coroutine(), boost::asio::detached);
    boost::asio::co_spawn(main_io_context, normal_in(), boost::asio::detached);

    return 0;
}