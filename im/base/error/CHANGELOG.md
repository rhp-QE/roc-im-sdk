# 错误处理系统变更日志

## 版本 1.0.0 - 2025-01-XX

### 新增功能

#### 1. 简化的Error类 (`im/base/error/Error.h`)

创建了一个轻量级的错误处理类，包含以下特性：

- **基本错误信息**: 错误码、错误消息、详细信息
- **兼容性**: 支持从std::string构造，支持隐式转换为std::string
- **流输出**: 支持直接输出到std::ostream
- **类型安全**: 基于C++23的std::expected

```cpp
// 基本用法
auto error = make_error(1001, "Network error", "Connection timeout");
std::cout << error.to_string() << std::endl;

// 兼容性用法
auto string_error = make_error("Simple error message");
std::string error_string = error; // 隐式转换
```

#### 2. 便捷函数

```cpp
// 创建错误
Error make_error(int code, const std::string& message);
Error make_error(int code, const std::string& message, const std::string& details);
Error make_error(const std::string& message); // 兼容性函数
```

### 修改的文件

#### 1. IWSClient.h (`im/base/lc/Interface/IWSClient.h`)

**修改内容:**
- 添加了Error.h头文件包含
- 将所有`std::expected<T, std::string>`返回类型改为`std::expected<T, roc::error::Error>`
- 修改了以下方法的返回类型：
  - `connect()`
  - `close()`
  - `send()`
  - `ping()`
  - `pong()`

**影响范围:**
- 所有继承IWSClient的类都需要相应修改
- 调用这些方法的代码需要更新错误处理逻辑

#### 2. WSClient.h (`im/base/lc/impl/ws/WSClient.h`)

**修改内容:**
- 修改了所有实现方法的返回类型声明
- 更新了方法文档注释

**修改的方法:**
- `connect_impl()`
- `close_impl()`
- `send_impl()`
- `ping_impl()`
- `pong_impl()`

#### 3. WSClient.cc (`im/base/lc/impl/ws/WSClient.cc`)

**修改内容:**
- 添加了Error.h头文件包含
- 修改了所有实现方法的返回类型
- 将所有错误创建从`std::string`改为`roc::error::Error`
- 更新了错误码分配

**错误码分配:**
- 1001: Invalid configuration
- 1002: Connection failed
- 1003: Not connected (close)
- 1004: Close failed
- 1005: Not connected (send)
- 1006: Invalid data or size
- 1007: Send failed
- 1008: Not connected (ping)
- 1009: Ping failed
- 1010: Not connected (pong)
- 1011: Pong failed

#### 4. LongConnectionClient.h (`im/base/lc/impl/ws/LongConnectionClient.h`)

**修改内容:**
- 修改了公共接口方法的返回类型
- 更新了方法文档注释

**修改的方法:**
- `connect()`
- `disconnect()`
- `send_data()`
- `send_message()`

#### 5. LongConnectionClient.cc (`im/base/lc/impl/ws/LongConnectionClient.cc`)

**修改内容:**
- 添加了Error.h头文件包含
- 修改了所有实现方法的返回类型
- 将所有错误创建从`std::string`改为`roc::error::Error`
- 更新了错误处理逻辑，使用`result.error().to_string()`获取错误字符串

**错误码分配:**
- 2001: Already connected
- 2002: Client is already running
- 2003: Failed to connect
- 2004: Connection failed (exception)
- 2005: Not connected (disconnect)
- 2006: Disconnect failed
- 2007: Disconnect failed (exception)
- 2008: Not connected (send)
- 2009: Invalid data or size
- 2010: Send failed
- 2011: Send failed (exception)

### 兼容性说明

#### 向后兼容性

1. **Error类兼容性**: Error类提供了从std::string构造的构造函数和隐式转换为std::string的操作符，确保现有代码可以平滑迁移。

2. **错误处理**: 所有错误现在都包含错误码，提供更详细的错误信息。

#### 迁移指南

1. **错误处理代码更新**:
   ```cpp
   // 旧代码
   auto result = client->connect();
   if (!result) {
       std::cout << "Error: " << result.error() << std::endl;
   }
   
   // 新代码
   auto result = client->connect();
   if (!result) {
       std::cout << "Error: " << result.error().to_string() << std::endl;
       // 或者直接使用流操作符
       std::cout << "Error: " << result.error() << std::endl;
   }
   ```

2. **错误创建**:
   ```cpp
   // 旧代码
   return std::unexpected("Connection failed");
   
   // 新代码
   return std::unexpected(roc::error::make_error(1001, "Connection failed"));
   ```

### 测试

#### 新增测试文件

1. **error_integration_test.cpp**: 测试Error类与WebSocket客户端的集成
2. **simple_error_test.cpp**: 测试简化版Error类的基本功能

#### 测试内容

- Error类的基本功能
- 从std::string构造和转换
- WebSocket客户端配置
- 长连接客户端配置
- 错误码范围测试

### 性能影响

- **内存使用**: Error类比std::string稍大，但提供了更多信息
- **运行时性能**: 影响微乎其微，主要是构造和字符串转换的开销
- **编译时间**: 由于添加了头文件包含，编译时间略有增加

### 未来计划

1. **错误码标准化**: 建立统一的错误码规范
2. **错误分类**: 按模块和功能分类错误码
3. **国际化支持**: 支持多语言错误消息
4. **错误统计**: 添加错误统计和监控功能 