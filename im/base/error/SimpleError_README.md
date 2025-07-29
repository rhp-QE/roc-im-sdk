# 简化的C++错误处理类

这是一个轻量级的C++错误处理类，只包含核心的错误信息功能。

## 特性

- 🚀 **轻量级**: 只包含必要的错误信息
- 📝 **简单易用**: 清晰的API设计
- 🔧 **类型安全**: 基于C++23的`std::expected`
- ⚡ **高性能**: 最小化内存开销

## 快速开始

### 基本用法

```cpp
#include "im/base/error/SimpleError.h"

using namespace roc::error;

// 创建错误
auto error = make_error(500, "Internal server error");
std::cout << error.to_string() << std::endl;
// 输出: Error[500]: Internal server error

// 带详细信息的错误
auto detailed_error = make_error(1001, "Network timeout", "Connection failed after 30 seconds");
std::cout << detailed_error.to_string() << std::endl;
// 输出: Error[1001]: Network timeout - Connection failed after 30 seconds
```

### 使用Result类型

```cpp
Result<int> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected(make_error(1001, "Division by zero", 
            "Attempted to divide " + std::to_string(a) + " by " + std::to_string(b)));
    }
    return success(a / b);
}

// 使用函数
auto result = divide(10, 2);
if (result) {
    std::cout << "Result: " << result.value() << std::endl;
} else {
    std::cout << "Error: " << result.error().to_string() << std::endl;
}
```

### 错误处理宏

```cpp
Result<int> process_data(int value) {
    RETURN_ERROR_IF(value < 0, make_error(3001, "Value must be non-negative"));
    
    auto div_result = divide(value, 2);
    RETURN_IF_ERROR(div_result);
    
    return success(div_result.value() * 2);
}
```

## API参考

### SimpleError类

```cpp
class SimpleError {
public:
    // 构造函数
    SimpleError();
    SimpleError(int code, std::string message);
    SimpleError(int code, std::string message, std::string details);
    
    // 获取器
    int code() const;
    const std::string& message() const;
    const std::string& details() const;
    const std::source_location& location() const;
    
    // 方法
    SimpleError with_details(const std::string& details) const;
    std::string to_string() const;
};
```

### 便捷函数

```cpp
// 创建错误
SimpleError make_error(int code, const std::string& message);
SimpleError make_error(int code, const std::string& message, const std::string& details);

// 创建结果
template<typename T>
Result<T> success(T&& value);
template<typename T>
Result<T> failure(const SimpleError& error);
VoidResult success();
VoidResult failure(const SimpleError& error);
```

### 类型别名

```cpp
template<typename T>
using Result = std::expected<T, SimpleError>;

using VoidResult = Result<void>;
```

### 宏

```cpp
// 如果条件为真，返回错误
RETURN_ERROR_IF(condition, error)

// 如果表达式返回错误，立即返回
RETURN_IF_ERROR(expr)

// 尝试表达式，如果失败则返回错误
TRY(expr)
```

## 使用示例

### 1. 基本错误处理

```cpp
Result<std::string> get_user_name(int user_id) {
    if (user_id < 0) {
        return std::unexpected(make_error(2001, "Invalid user ID", 
            "User ID must be positive"));
    }
    
    if (user_id > 1000) {
        return std::unexpected(make_error(2002, "User not found", 
            "User with ID " + std::to_string(user_id) + " does not exist"));
    }
    
    return success("User " + std::to_string(user_id));
}
```

### 2. 链式调用

```cpp
Result<int> complex_operation(int input) {
    RETURN_ERROR_IF(input < 0, make_error(3001, "Input must be non-negative"));
    
    auto step1_result = step1(input);
    RETURN_IF_ERROR(step1_result);
    
    auto step2_result = step2(step1_result.value());
    RETURN_IF_ERROR(step2_result);
    
    return success(step2_result.value());
}
```

### 3. 流输出

```cpp
auto error = make_error(4001, "Test error", "This is a test");
std::cout << error << std::endl;
// 输出: Error[4001]: Test error - This is a test
```

## 编译要求

- C++23 或更高版本
- 支持 `std::expected` 的编译器

## 示例

查看 `test/simple_error_test.cpp` 文件获取完整的使用示例。

## 与完整版本的对比

| 功能 | SimpleError | 完整版Error |
|------|-------------|-------------|
| 错误码 | ✅ | ✅ |
| 错误消息 | ✅ | ✅ |
| 详细信息 | ✅ | ✅ |
| 错误类型 | ❌ | ✅ |
| 错误严重程度 | ❌ | ✅ |
| 错误上下文 | ❌ | ✅ |
| 堆栈跟踪 | ❌ | ✅ |
| 链式错误 | ❌ | ✅ |
| 多种格式化 | ❌ | ✅ |
| 错误搜索 | ❌ | ✅ |
| 自定义格式化器 | ❌ | ✅ |

SimpleError版本专注于核心功能，适合需要轻量级错误处理的场景。 