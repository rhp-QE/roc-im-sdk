#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <type_traits>
#include <array>
#include <iostream>

namespace roc::imsdk {

/**
 * 日志级别枚举
 * 由外界使用者设置，不需要注入
 */
enum class LogLevel {
    Trace    = 0,
    Debug    = 1,
    Info     = 2,
    Warn     = 3,
    Error    = 4,
    Critical = 5,
    Off      = 6,
};



// 消息格式化函数实现 ========================================================================
// 无参数版本的fmt风格格式化
inline std::string format_message_fmt(const char* format) {
    if (!format) return "";
    return std::string(format);
}

// Helper function to convert various types to string
template<typename T>
std::string to_string(const T& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<T, const char*>) {
        return std::string(value);
    } else if constexpr (std::is_same_v<T, char*>) {
        return std::string(value);
    } else if constexpr (std::is_same_v<T, char>) {
        return std::string(1, value);
    } else if constexpr (std::is_array_v<T> && 
                        (std::is_same_v<std::remove_extent_t<T>, char> || 
                         std::is_same_v<std::remove_extent_t<T>, const char>)) {
        // 处理字符串字面量 (如 "hello" -> const char[6])
        return std::string(value);
    } else if constexpr (std::is_integral_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(value);
    } else {
        // 对于其他类型，尝试使用 std::to_string，如果失败则返回类型名
        try {
            return std::to_string(value);
        } catch (...) {
            return "[Unknown Type]";
        }
    }
}

template<typename... Args>
std::string format_message_fmt(const char* format, Args&&... args) {
    if (!format) return "";
    
    std::string result = format;
    std::vector<std::string> values = {to_string(std::forward<Args>(args))...};
    
    size_t pos = 0;
    size_t argIndex = 0;
    
    while ((pos = result.find("{}", pos)) != std::string::npos && argIndex < values.size()) {
        result.replace(pos, 2, values[argIndex]);
        pos += values[argIndex].length();
        argIndex++;
    }
    
    return result;
}
// ========================================================================================

class ILogger {
public:
    using LogFunction = std::function<void(LogLevel, const std::string&, const std::string&)>;
    
    using LevelSetter = std::function<void(LogLevel)>;
    
    using LevelGetter = std::function<LogLevel()>;
    
    using LevelChecker = std::function<bool(LogLevel)>;
    
    using FlushHandler = std::function<void()>;

    ILogger() = default;
    virtual ~ILogger() = default;

    void set_log_function(LogFunction log_func) { log_func_ = std::move(log_func); }
    
    void set_level_function(LevelSetter func) { set_level_func_ = std::move(func); }
    
    void set_get_level_function(LevelGetter func) { get_level_func_ = std::move(func); }
    
    void set_should_log_function(LevelChecker func) { should_log_func_ = std::move(func); }
    
    void set_flush_function(FlushHandler func) { flush_func_ = std::move(func); }

    void log(LogLevel level, const std::string& module, const std::string& message) {
        if (log_func_) {
            log_func_(level, module, message);
        }
    }
    
    void set_level(LogLevel level) {
        if (set_level_func_) {
            set_level_func_(level);
        }
    }
    
    LogLevel get_level() const {
        if (get_level_func_) {
            return get_level_func_();
        }
        return LogLevel::Info;
    }
    
    bool should_log(LogLevel level) const {
        if (should_log_func_) {
            return should_log_func_(level);
        }
        return level >= get_level();
    }
    
    void flush() {
        if (flush_func_) {
            flush_func_();
        }
    }

    // Template method for fmt-style formatting with {} placeholders
    template<typename... Args>
    void log_fmt(LogLevel level, const std::string& module, const char* format, Args&&... args) {
        std::string message = roc::imsdk::format_message_fmt(format, std::forward<Args>(args)...);
        log(level, module, message);
    }

private:
    LogFunction log_func_;
    LevelSetter set_level_func_;
    LevelGetter get_level_func_;
    LevelChecker should_log_func_;
    FlushHandler flush_func_;
};


} // namespace roc_im_sdk
