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

/**
 * Spdlog 适配器
 * 内部直接创建和配置 spdlog，设置输出目录等逻辑
 * 只暴露 GetLogger() 方法给外界
 */
class SpdlogAdapter : public std::enable_shared_from_this<SpdlogAdapter> {
public:
    /**
     * 构造函数 - 使用默认配置
     * 自动创建控制台和文件日志器
     */
    SpdlogAdapter();
    
    /**
     * 构造函数 - 指定日志目录
     * @param logDir 日志输出目录
     */
    explicit SpdlogAdapter(const std::string& logDir);
    
    /**
     * 构造函数 - 从现有的 spdlog 日志器创建
     * @param logger 现有的 spdlog 日志器
     */
    explicit SpdlogAdapter(std::shared_ptr<spdlog::logger> logger);
    
    /**
     * 析构函数
     */
    ~SpdlogAdapter();
    
    /**
     * 获取配置好的 ILogger 实例
     * @return 配置好的 ILogger 实例
     */
    std::shared_ptr<ILogger> get_logger() const;
    
    /**
     * 设置日志级别
     * @param level 日志级别
     */
    void set_level(LogLevel level);
    
    /**
     * 获取当前日志级别
     * @return 当前日志级别
     */
    LogLevel get_level() const;
    
    /**
     * 刷新日志缓冲区
     */
    void flush();

    /**
     * 测试异步日志和 {} 占位符格式化功能
     */
    void test_async_logging() const;

    /**
     * 启动定期日志刷新（使用协程）
     * @param io_context boost::asio io_context
     * @param interval_ms 刷新间隔（毫秒），默认1000ms
     */
    void start_periodic_flush_async(std::shared_ptr<boost::asio::io_context> io_context, int interval_ms = 1000);

    /**
     * 停止定期日志刷新
     */
    void stop_periodic_flush();

    /**
     * 检查定期刷新是否正在运行
     */
    bool is_periodic_flush_running() const;

    /**
     * 创建 SpdlogAdapter 的静态工厂函数
     * @param logDir 日志输出目录（可选）
     * @return SpdlogAdapter 实例
     */
    static std::shared_ptr<SpdlogAdapter> create(const std::string& logDir = "logs");
    
    /**
     * 从现有的 spdlog 日志器创建适配器的静态工厂函数
     * @param logger 现有的 spdlog 日志器
     * @return SpdlogAdapter 实例
     */
    static std::shared_ptr<SpdlogAdapter> create(std::shared_ptr<spdlog::logger> logger);

private:
    std::shared_ptr<spdlog::logger> spdlogLogger_;
    std::string logDir_;
    
    // 定期刷新相关
    std::shared_ptr<boost::asio::io_context> io_context_;
    std::atomic<bool> stop_flush_;
    std::atomic<bool> flush_running_;
    
    /**
     * 初始化默认的 spdlog 配置
     */
    void initialize_default_logger();
    
    /**
     * 创建日志目录
     */
    void create_log_directory();
    
    /**
     * 定期刷新协程函数
     */
    boost::asio::awaitable<void> periodic_flush_coroutine(int interval_ms);
    
    /**
     * 将我们的 LogLevel 转换为 spdlog 的 level
     */
    static spdlog::level::level_enum convert_level(LogLevel level);
    
    /**
     * 将 spdlog 的 level 转换为我们的 LogLevel
     */
    static LogLevel convert_level(spdlog::level::level_enum level);
};

} // namespace roc::imsdk
