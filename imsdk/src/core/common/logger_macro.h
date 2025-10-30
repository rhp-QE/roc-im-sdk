#pragma once

#include <string>
// MSVC 使用 __FUNCSIG__，GCC/Clang 使用 __PRETTY_FUNCTION__
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// 检测是否输出到终端
inline bool need_color() {
    // isatty 是 POSIX 函数，暂时禁用
    // return isatty(fileno(stdout));
    return false;
}

// 条件颜色宏
#define LOG_COLOR_RESET   (need_color() ? "\033[0m" : "")
#define LOG_COLOR_RED     (need_color() ? "\033[31m" : "")
#define LOG_COLOR_GREEN   (need_color() ? "\033[32m" : "")
#define LOG_COLOR_YELLOW  (need_color() ? "\033[33m" : "")
#define LOG_COLOR_BLUE    (need_color() ? "\033[34m" : "")
#define LOG_COLOR_MAGENTA (need_color() ? "\033[35m" : "")
#define LOG_COLOR_CYAN    (need_color() ? "\033[36m" : "")
#define LOG_COLOR_WHITE   (need_color() ? "\033[37m" : "")
#define LOG_COLOR_BOLD    (need_color() ? "\033[1m" : "")

inline std::string extract_class_method(const std::string& pretty_function) {
    std::string result = pretty_function;
    
    // 移除返回类型
    size_t pos = result.find('(');
    if (pos != std::string::npos) {
        result = result.substr(0, pos);
    }
    
    // 查找最后一个::，提取类名和方法名
    pos = result.rfind("::");
    if (pos != std::string::npos) {
        // 查找类名开始位置（最后一个::之前）
        size_t class_start = result.rfind("::", pos - 1);
        if (class_start != std::string::npos) {
            // 有命名空间，提取类名和方法名
            size_t class_name_start = class_start + 2;
            std::string class_name = result.substr(class_name_start, pos - class_name_start);
            std::string method_name = result.substr(pos + 2);
            return class_name + "::" + method_name;
        } else {
            // 没有命名空间，直接提取
            return result.substr(pos + 2);
        }
    }
    return result;
}

inline std::string extract_filename(const std::string& file_path) {
    size_t pos = file_path.find_last_of("/\\");
    return (pos != std::string::npos) ? file_path.substr(pos + 1) : file_path;
}

#define LOG_TRACE(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Trace)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Trace, im_sdk_log_context_macro, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_DEBUG(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Debug)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Debug, im_sdk_log_context_macro,"\n    track_id = {}, " format "\n", TRACK_ID, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_INFO(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Info)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Info, im_sdk_log_context_macro, "\n    track_id = {}, " format "\n", TRACK_ID, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_WARN(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Warn)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Warn, im_sdk_log_context_macro, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_ERROR(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Error)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Error, im_sdk_log_context_macro, format, ##__VA_ARGS__); \
     } \
 } while(0);

#define LOG_CRITICAL(module, format, ...) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Critical)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Critical, im_sdk_log_context_macro, format, ##__VA_ARGS__); \
     } \
 } while(0);

// 性能日志宏（包含时间戳）
#define LOG_PERF(module, operation, duration_ms) \
 do { \
     if (sdk_root && sdk_root->logger()->should_log(roc::imsdk::LogLevel::Info)) { \
         std::string im_sdk_log_context_macro = LOG_COLOR_CYAN + std::string("<") + extract_class_method(__PRETTY_FUNCTION__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_GREEN + "<" + extract_filename(__FILE__) + ":" + std::to_string(__LINE__) + ">" + LOG_COLOR_RESET + \
                                              LOG_COLOR_YELLOW + "<" + std::string(module) + ">" + LOG_COLOR_RESET; \
         sdk_root->logger()->log_fmt(roc::imsdk::LogLevel::Info, im_sdk_log_context_macro, "PERF: {} took {}ms", operation, duration_ms); \
     } \
 } while(0);
