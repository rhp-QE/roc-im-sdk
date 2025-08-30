#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <filesystem>
#include <iostream>

namespace roc_im_sdk {

// 静态成员初始化
bool Logger::initialized_ = false;
std::string Logger::logDir_ = "logs";
std::string Logger::currentPattern_ = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";
std::shared_ptr<spdlog::logger> Logger::defaultLogger_ = nullptr;

void Logger::Initialize(const std::string& logDir, Level logLevel, bool enableConsole, bool enableFile) {
    if (initialized_) {
        return;
    }

    logDir_ = logDir;
    CreateLogDirectory(logDir_);

    std::vector<spdlog::sink_ptr> sinks;

    // 添加控制台输出
    if (enableConsole) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        sinks.push_back(console_sink);
    }

    // 添加文件输出
    if (enableFile) {
        // 轮转文件日志
        auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logDir_ + "/roc_im_sdk.log",
            1024 * 1024 * 10, // 10MB
            5                   // 保留5个文件
        );
        rotating_sink->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        sinks.push_back(rotating_sink);

        // 每日文件日志
        auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            logDir_ + "/roc_im_sdk_daily.log",
            0, 0  // hour and minute for rotation
        );
        daily_sink->set_level(static_cast<spdlog::level::level_enum>(logLevel));
        sinks.push_back(daily_sink);
    }

    // 创建默认日志器
    defaultLogger_ = std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
    defaultLogger_->set_level(static_cast<spdlog::level::level_enum>(logLevel));
    defaultLogger_->set_pattern(GetDefaultPattern());

    // 注册到 spdlog
    spdlog::register_logger(defaultLogger_);
    spdlog::set_default_logger(defaultLogger_);

    // 设置全局错误处理器
    spdlog::set_error_handler([](const std::string& msg) {
        std::cerr << "spdlog error: " << msg << std::endl;
    });

    initialized_ = true;
    
    LOG_INFO("日志系统初始化完成，日志目录: {}", logDir_);
}

void Logger::Shutdown() {
    if (!initialized_) {
        return;
    }

    LOG_INFO("正在关闭日志系统...");
    spdlog::shutdown();
    
    initialized_ = false;
    defaultLogger_ = nullptr;
}

std::shared_ptr<spdlog::logger> Logger::GetLogger(const std::string& name) {
    if (!initialized_) {
        // 如果未初始化，使用默认初始化
        Initialize();
    }

    if (name == "default") {
        return defaultLogger_;
    }

    auto logger = spdlog::get(name);
    if (!logger) {
        // 创建新的日志器
        logger = std::make_shared<spdlog::logger>(name, defaultLogger_->sinks().begin(), defaultLogger_->sinks().end());
        logger->set_level(defaultLogger_->level());
        logger->set_pattern(currentPattern_);
        spdlog::register_logger(logger);
    }

    return logger;
}

void Logger::SetLevel(Level level) {
    if (!initialized_) {
        return;
    }

    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
    if (defaultLogger_) {
        defaultLogger_->set_level(static_cast<spdlog::level::level_enum>(level));
    }
}

void Logger::SetPattern(const std::string& pattern) {
    if (!initialized_) {
        return;
    }

    spdlog::set_pattern(pattern);
    if (defaultLogger_) {
        defaultLogger_->set_pattern(pattern);
    }
}

void Logger::Flush() {
    if (initialized_) {
        // spdlog doesn't have flush_all, individual loggers can be flushed if needed
    }
}

void Logger::CreateLogDirectory(const std::string& logDir) {
    try {
        std::filesystem::create_directories(logDir);
    } catch (const std::exception& e) {
        std::cerr << "无法创建日志目录 " << logDir << ": " << e.what() << std::endl;
    }
}

std::string Logger::GetDefaultPattern() {
    return "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";
}

} // namespace roc_im_sdk
