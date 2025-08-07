#ifndef ROCIM_ERROR_H
#define ROCIM_ERROR_H

#include <string>
#include <memory>

namespace roc::error {

// 简化的错误类
class Error : public std::enable_shared_from_this<Error> {
private:
    int code_;
    std::string message_;
    std::string details_;

public:
    // 构造函数
    Error() : code_(0) {}
    
    Error(int code, std::string message) 
        : code_(code), message_(std::move(message)) {}
    
    Error(int code, std::string message, std::string details)
        : code_(code), message_(std::move(message)), details_(std::move(details)) {}
    
    // 从std::string构造（兼容性）
    Error(const std::string& message) : code_(0), message_(message) {}
    
    // 获取器
    int code() const { return code_; }
    const std::string& message() const { return message_; }
    const std::string& details() const { return details_; }
    
    // 转换为字符串
    std::string to_string() const {
        if (code_ == 0) {
            return message_;
        }
        std::string result = "Error[" + std::to_string(code_) + "]: " + message_;
        if (!details_.empty()) {
            result += " - " + details_;
        }
        return result;
    }
    
    // 隐式转换为std::string（兼容性）
    operator std::string() const {
        return to_string();
    }
};

// 便捷创建函数
inline Error make_error(int code, const std::string& message) {
    return Error(code, message);
}

inline Error make_error(int code, const std::string& message, const std::string& details) {
    return Error(code, message, details);
}

// 从std::string创建Error（兼容性）
inline Error make_error(const std::string& message) {
    return Error(message);
}

} // namespace roc::error

// 流操作符重载
inline std::ostream& operator<<(std::ostream& os, const roc::error::Error& error) {
    os << error.to_string();
    return os;
}

#endif // ROCIM_ERROR_H 