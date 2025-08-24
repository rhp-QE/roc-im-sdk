#include <boost/asio/detail/concurrency_hint.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <memory>
#include <test/buffertest.h>
#include <test/corTest.h>
#include <thread>

#include "BaseConfig.h"
#include "test/mmkv_test.h"
#include "test/test_func_.h"
#include <im/base/coroutine.h>
#include <test/boostCoroTest.h>
#include <test/channelTest.h>
#include <test/coCostTime.h>
#include <test/httpTest.h>
#include <test/testwc.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <expected>
#include <test/wsclientTest.h>
#include <test/imsdk_test.h>
#include <test/wcdb_test.h>
#include <test/wcdb_simple_test.h>

boost::asio::io_context net_io_context{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE_IO};
boost::asio::io_context main_io_context{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE_IO};
boost::asio::io_context sdk_io_context{BOOST_ASIO_CONCURRENCY_HINT_UNSAFE_IO};

namespace roc::coro {
std::unordered_map<size_t, std::shared_ptr<CoroRAII>> coro_manager;
std::mutex coro_mutex;
} // namespace roc::coro


std::expected<int, std::string> testex() {
    return std::unexpected("123");
}

template <typename Ty> void print(Ty aa) {
    Ty v = aa;
    std::cout << v << std::endl;
}

template <size_t... I> void print_index(std::index_sequence<I...>) {
    (..., print(I));
}

template <typename... Retype> void test_tuple(std::tuple<Retype...> res) {
    auto index =
        std::make_index_sequence<std::tuple_size<decltype(res)>::value>{};
    [&]<size_t... I>(std::index_sequence<I...>) {
        (..., (std::cout << std::get<I>(res) << std::endl));
    }(index);
}

struct Node {
    ~Node() { std::cout << "faf" << std::endl; }
};

int gcd(int a, int b) {
    if (b == 0) {
        return a;
    }
    return gcd(b, a % b);
}

void test_gcd() {
    std::cout << gcd(10, 15) << std::endl;
}

int main() {

    // testChannel();

    // test_func_();

    // test_mmkv();

    // // 运行 WCDB 测试
    // run_wcdb_tests();
    
    // 运行简化的 WCDB 测试（严格按照官方教程）
    // wcdb_simple_test();

    test_imsdk();

    // co_cost_time_test();

    // boostCoroTest();
    // testWC();
    // testChannel();
    // wsclientTestMain();
    // grpc_client_test();

    auto wark_work = boost::asio::make_work_guard(net_io_context);
    std::thread net_thread([] { net_io_context.run(); });

    auto wark_work1 = boost::asio::make_work_guard(sdk_io_context);
    std::thread net_thread1([] { sdk_io_context.run(); });

    auto main_work = boost::asio::make_work_guard(main_io_context);
    main_io_context.run();

    return 0;
}
