#include "ILogger.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <vector>
#include <type_traits>

namespace roc::imsdk {

static std::shared_ptr<ILogger> g_logger = nullptr;

// 默认日志函数实现
void DefaultLogFunction(LogLevel level, const std::string& module, const std::string& message) {
    std::string levelStr;
    switch (level) {
        case LogLevel::Trace: levelStr = "TRACE"; break;
        case LogLevel::Debug: levelStr = "DEBUG"; break;
        case LogLevel::Info: levelStr = "INFO"; break;
        case LogLevel::Warn: levelStr = "WARN"; break;
        case LogLevel::Error: levelStr = "ERROR"; break;
        case LogLevel::Critical: levelStr = "CRITICAL"; break;
        default: levelStr = "UNKNOWN"; break;
    }
    std::cout << "[" << levelStr << "] [" << module << "] " << message << std::endl;
}

// 默认级别设置函数
void DefaultSetLevelFunction(LogLevel level) {
    // 默认实现：不做任何操作
    (void)level;
}

// 默认级别获取函数
LogLevel DefaultGetLevelFunction() {
    return LogLevel::Info;
}

// 默认级别检查函数
bool DefaultShouldLogFunction(LogLevel level) {
    return level >= LogLevel::Info;
}

// 默认刷新函数
void DefaultFlushFunction() {
    std::cout.flush();
}

// 创建默认日志器
std::shared_ptr<ILogger> CreateDefaultLogger() {
    auto logger = std::make_shared<ILogger>();
    logger->set_log_function(DefaultLogFunction);
    logger->set_level_function(DefaultSetLevelFunction);
    logger->set_get_level_function(DefaultGetLevelFunction);
    logger->set_should_log_function(DefaultShouldLogFunction);
    logger->set_flush_function(DefaultFlushFunction);
    return logger;
}



// 全局日志器访问函数实现
std::shared_ptr<ILogger> GetLogger() {
    if (!g_logger) {
        g_logger = CreateDefaultLogger();
    }
    return g_logger;
}

void SetLogger(std::shared_ptr<ILogger> logger) {
    g_logger = std::move(logger);
}



} // namespace roc_im_sdk
