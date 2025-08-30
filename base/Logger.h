#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <memory>
#include <string>

namespace roc_im_sdk {

/**
 * ROC IM SDK 日志系统
 * 基于 spdlog 的统一日志接口
 */
class Logger {
public:
    enum class Level {
        Trace = spdlog::level::trace,
        Debug = spdlog::level::debug,
        Info = spdlog::level::info,
        Warn = spdlog::level::warn,
        Error = spdlog::level::err,
        Critical = spdlog::level::critical,
        Off = spdlog::level::off
    };

    /**
     * 初始化日志系统
     * @param logDir 日志文件目录
     * @param logLevel 日志级别
     * @param enableConsole 是否启用控制台输出
     * @param enableFile 是否启用文件输出
     */
    static void Initialize(const std::string& logDir = "logs",
                          Level logLevel = Level::Info,
                          bool enableConsole = true,
                          bool enableFile = true);

    /**
     * 清理日志系统
     */
    static void Shutdown();

    /**
     * 获取日志器实例
     * @param name 日志器名称
     * @return 日志器指针
     */
    static std::shared_ptr<spdlog::logger> GetLogger(const std::string& name = "default");

    /**
     * 设置全局日志级别
     * @param level 日志级别
     */
    static void SetLevel(Level level);

    /**
     * 设置日志格式
     * @param pattern 格式模式
     */
    static void SetPattern(const std::string& pattern);

    /**
     * 刷新所有日志
     */
    static void Flush();

private:
    static bool initialized_;
    static std::string logDir_;
    static std::string currentPattern_;
    static std::shared_ptr<spdlog::logger> defaultLogger_;
    
    static void CreateLogDirectory(const std::string& logDir);
    static std::string GetDefaultPattern();
};

// 便捷的日志宏
#define LOG_TRACE(...)    SPDLOG_LOGGER_TRACE(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)
#define LOG_DEBUG(...)    SPDLOG_LOGGER_DEBUG(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)
#define LOG_INFO(...)     SPDLOG_LOGGER_INFO(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)
#define LOG_WARN(...)     SPDLOG_LOGGER_WARN(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)
#define LOG_ERROR(...)    SPDLOG_LOGGER_ERROR(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)
#define LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(roc_im_sdk::Logger::GetLogger(), __VA_ARGS__)

// 带模块名的日志宏
#define LOG_MODULE_TRACE(module, ...)    SPDLOG_LOGGER_TRACE(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)
#define LOG_MODULE_DEBUG(module, ...)    SPDLOG_LOGGER_DEBUG(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)
#define LOG_MODULE_INFO(module, ...)     SPDLOG_LOGGER_INFO(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)
#define LOG_MODULE_WARN(module, ...)     SPDLOG_LOGGER_WARN(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)
#define LOG_MODULE_ERROR(module, ...)    SPDLOG_LOGGER_ERROR(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)
#define LOG_MODULE_CRITICAL(module, ...) SPDLOG_LOGGER_CRITICAL(roc_im_sdk::Logger::GetLogger(module), __VA_ARGS__)

} // namespace roc_im_sdk
