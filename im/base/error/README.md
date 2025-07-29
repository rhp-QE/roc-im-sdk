# C++ Error Handling Library

这是一个现代化的C++错误处理库，提供了丰富的错误信息、链式错误、格式化输出等功能。

## 特性

- 🚀 **现代化设计**: 基于C++23的`std::expected`，类型安全
- 🔗 **链式错误**: 支持错误链，便于追踪错误传播
- 📝 **丰富信息**: 包含错误码、消息、详情、上下文、堆栈跟踪等
- 🎨 **多种格式**: 支持默认、紧凑、JSON等多种输出格式
- 🛠️ **便捷宏**: 提供错误处理宏，简化代码
- 🔍 **错误搜索**: 支持在错误链中查找特定类型的错误
- ⚡ **高性能**: 零拷贝设计，高效的内存使用

## 快速开始

### 基本用法

```cpp
#include "im/base/error/Error.h"

using namespace roc::error;

// 创建错误
auto error = make_system_error(500, "Internal server error");

// 添加详细信息
error = error.with_details("Database connection failed")
             .with_context(ErrorContext("user_login", "auth_service"))
             .with_suggestion("Check database status")
             .with_stack_trace();

// 输出错误信息
std::cout << error.to_string() << std::endl;
```

### 使用Result类型

```cpp
Result<int> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected(make_validation_error(1001, "Division by zero")
            .with_details("Attempted to divide " + std::to_string(a) + " by " + std::to_string(b))
            .with_stack_trace());
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
    RETURN_ERROR_IF(value < 0, make_validation_error(1002, "Value must be non-negative"));
    
    auto result = some_operation(value);
    RETURN_IF_ERROR(result);
    
    return success(result.value() * 2);
}
```

## 错误类型

### 预定义错误类型

- `ErrorType::System` - 系统错误
- `ErrorType::Network` - 网络错误
- `ErrorType::Database` - 数据库错误
- `ErrorType::Validation` - 验证错误
- `ErrorType::Business` - 业务逻辑错误
- `ErrorType::Configuration` - 配置错误
- `ErrorType::Timeout` - 超时错误
- `ErrorType::Resource` - 资源错误
- `ErrorType::Security` - 安全错误

### 错误严重程度

- `ErrorSeverity::Info` - 信息
- `ErrorSeverity::Warning` - 警告
- `ErrorSeverity::Error` - 错误
- `ErrorSeverity::Critical` - 严重错误
- `ErrorSeverity::Fatal` - 致命错误

## 便捷函数

### 错误创建函数

```cpp
// 系统错误
auto sys_error = make_system_error(500, "Internal error");

// 网络错误
auto net_error = make_network_error(1001, "Connection timeout");

// 数据库错误
auto db_error = make_database_error(2001, "Query failed");

// 验证错误
auto val_error = make_validation_error(3001, "Invalid input");

// 业务错误
auto bus_error = make_business_error(4001, "User not found");

// 超时错误
auto timeout_error = make_timeout_error(5001, "Operation timeout");
```

### 结果创建函数

```cpp
// 成功结果
auto success_result = success(42);
auto void_success = success();

// 失败结果
auto failure_result = failure(make_system_error(500, "Error"));
auto void_failure = failure(make_system_error(500, "Error"));
```

## 错误格式化

### 内置格式化器

```cpp
auto error = make_network_error(1001, "Connection failed");

// 默认格式
std::cout << error.to_string() << std::endl;

// 紧凑格式
std::cout << error.format_error("compact") << std::endl;

// JSON格式
std::cout << error.format_error("json") << std::endl;
```

### 自定义格式化器

```cpp
Error::register_formatter("simple", [](const Error& error) {
    return std::format("ERROR {}: {}", error.code().code, error.message());
});

auto error = make_business_error(3001, "Custom error");
std::cout << error.format_error("simple") << std::endl;
```

## 链式错误

```cpp
// 创建链式错误
auto db_error = make_database_error(2001, "Connection failed");
auto wrapped_error = make_business_error(3001, "User operation failed")
    .with_cause(std::make_shared<Error>(db_error));

// 遍历错误链
wrapped_error.for_each_cause([](const Error& error) {
    std::cout << "Error: " << error.format_error("compact") << std::endl;
});

// 查找特定类型的错误
auto db_error_found = wrapped_error.find_if([](const Error& e) {
    return e.is_database();
});
```

## 错误上下文

```cpp
auto context = ErrorContext("user_authentication", "auth_service");
context.add_metadata("user_id", "12345");
context.add_metadata("ip_address", "192.168.1.1");

auto error = make_business_error(3001, "Authentication failed")
    .with_context(context);
```

## 错误码定义

### 系统错误码

```cpp
namespace error_codes {
    const ErrorCode SUCCESS(0, "system", "Success");
    const ErrorCode UNKNOWN_ERROR(1, "system", "Unknown error");
    const ErrorCode INVALID_ARGUMENT(2, "system", "Invalid argument");
    const ErrorCode OUT_OF_MEMORY(3, "system", "Out of memory");
    const ErrorCode PERMISSION_DENIED(4, "system", "Permission denied");
}
```

### 网络错误码

```cpp
namespace error_codes {
    const ErrorCode NETWORK_TIMEOUT(1001, "network", "Network timeout");
    const ErrorCode CONNECTION_REFUSED(1002, "network", "Connection refused");
    const ErrorCode CONNECTION_RESET(1003, "network", "Connection reset");
    const ErrorCode DNS_RESOLUTION_FAILED(1004, "network", "DNS resolution failed");
}
```

## 最佳实践

### 1. 使用有意义的错误码

```cpp
// 好的做法
return std::unexpected(make_validation_error(1001, "Invalid email format"));

// 避免
return std::unexpected(make_system_error(1, "Error"));
```

### 2. 提供详细的错误信息

```cpp
auto error = make_database_error(2001, "Query failed")
    .with_details("Failed to execute SELECT query on users table")
    .with_context(ErrorContext("get_user", "user_service"))
    .with_suggestion("Check database connection")
    .with_suggestion("Verify SQL syntax");
```

### 3. 使用链式错误追踪错误传播

```cpp
auto low_level_error = make_database_error(2001, "Connection failed");
auto mid_level_error = make_business_error(3001, "User data retrieval failed")
    .with_cause(std::make_shared<Error>(low_level_error));
auto high_level_error = make_system_error(500, "API request failed")
    .with_cause(std::make_shared<Error>(mid_level_error));
```

### 4. 使用宏简化错误处理

```cpp
Result<int> complex_operation(int input) {
    RETURN_ERROR_IF(input < 0, make_validation_error(1001, "Input must be positive"));
    
    auto step1_result = step1(input);
    RETURN_IF_ERROR(step1_result);
    
    auto step2_result = step2(step1_result.value());
    RETURN_IF_ERROR(step2_result);
    
    return success(step2_result.value());
}
```

## 编译要求

- C++23 或更高版本
- 支持 `std::expected` 的编译器

## 示例

查看 `test/error_test.cpp` 文件获取完整的使用示例。

## 许可证

本项目采用 MIT 许可证。 