#include <boost/asio/detail/concurrency_hint.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <memory>
#include <thread>

#include "BaseConfig.h"
#include <im/base/coroutine.h>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <expected>
#include <iostream>

// 保留有用的测试文件
#include <test/imsdk_test.h>
#include <examples/imsdk_demo.h>

#ifdef _WIN32
#include <windows.h>
#include <clocale>
#endif

// IO 上下文
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

int main(int argc, char *argv[]) {
#ifdef _WIN32
    // Windows 平台配置
    SetConsoleOutputCP(CP_UTF8);
    std::setlocale(LC_ALL, "zh_CN.UTF-8");
#endif

    std::cout << "=== ROCIM SDK Demo ===" << std::endl;
    std::cout << "SDK 初始化测试..." << std::endl;

    // 启动 IO 上下文工作线程
    auto net_work = boost::asio::make_work_guard(net_io_context);
    auto main_work = boost::asio::make_work_guard(main_io_context);
    auto sdk_work = boost::asio::make_work_guard(sdk_io_context);

    std::thread net_thread([&]() { 
        std::cout << "Net IO thread started" << std::endl;
        net_io_context.run(); 
        std::cout << "Net IO thread stopped" << std::endl;
    });
    
    std::thread sdk_thread([&]() { 
        std::cout << "SDK IO thread started" << std::endl;
        sdk_io_context.run(); 
        std::cout << "SDK IO thread stopped" << std::endl;
    });

    // 运行 IM SDK 示例
    try {
        std::cout << "\n=== 运行 IM SDK 示例 ===" << std::endl;
        // imsdk_demo 不返回 awaitable，直接调用
        imsdk_demo();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    // main_io_context 在主线程运行，直到程序结束
    std::cout << "\n主线程开始运行 main_io_context..." << std::endl;
    std::cout << "按 Ctrl+C 退出程序" << std::endl;
    
    // 主线程运行 main_io_context，阻塞直到所有工作完成或收到停止信号
    main_io_context.run();

    // 程序退出时清理
    std::cout << "\n正在停止其他 IO 上下文..." << std::endl;
    net_work.reset();
    sdk_work.reset();
    
    net_io_context.stop();
    sdk_io_context.stop();

    // 等待其他线程结束
    if (net_thread.joinable()) net_thread.join();
    if (sdk_thread.joinable()) sdk_thread.join();

    std::cout << "程序正常退出" << std::endl;
    return 0;
}
