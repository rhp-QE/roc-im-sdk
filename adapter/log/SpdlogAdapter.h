#pragma once

#include "imsdk/src/include/injection/log/ILogger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <boost/asio.hpp>

namespace roc::imsdk {


class SpdlogAdapter : public std::enable_shared_from_this<SpdlogAdapter> {
public:
    SpdlogAdapter();
    
    explicit SpdlogAdapter(const std::string& logDir);
    
    explicit SpdlogAdapter(std::shared_ptr<spdlog::logger> logger);
    
    ~SpdlogAdapter();
    
    std::shared_ptr<ILogger> get_logger() const;

    void start_periodic_flush_async(std::shared_ptr<boost::asio::io_context> io_context, int interval_ms = 1000);

    void stop_periodic_flush();

    bool is_periodic_flush_running() const;

    static std::shared_ptr<SpdlogAdapter> create(const std::string& logDir = "logs");
    
    static std::shared_ptr<SpdlogAdapter> create(std::shared_ptr<spdlog::logger> logger);

private:
    std::shared_ptr<spdlog::logger> spdlogLogger_;
    std::string logDir_;
    
    // 定期刷新相关
    std::shared_ptr<boost::asio::io_context> io_context_;
    std::atomic<bool> stop_flush_;
    std::atomic<bool> flush_running_;
    
    void initialize_default_logger();

    void create_log_directory();
    
    boost::asio::awaitable<void> periodic_flush_coroutine(int interval_ms);

    static spdlog::level::level_enum convert_level(LogLevel level);
    
    static LogLevel convert_level(spdlog::level::level_enum level);
};

} // namespace roc::imsdk
