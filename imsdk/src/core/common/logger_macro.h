#pragma once

#include "imsdk/src/include/injection/log/ILogger.h"

/**
 * 便捷的日志宏定义
 * 这些宏会调用注入的日志函数
 */
 #define LOG_TRACE(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Trace)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Trace, module, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_DEBUG(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Debug)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Debug, module, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_INFO(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Info)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Info, module, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_WARN(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Warn)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Warn, module, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_ERROR(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Error)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Error, module, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_CRITICAL(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Critical)) { \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Critical, module, format, ##__VA_ARGS__); \
     } \
 } while(0);