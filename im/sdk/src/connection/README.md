# SDKConnectionManager

IM SDK 连接管理器，负责管理 SDK 的长连接，底层使用 `LongConnectionClient`。

## 功能特性

- **连接生命周期管理**: 支持连接、断开、重连等操作
- **自动重连机制**: 连接断开时自动重连，支持退避策略
- **心跳保活**: 自动发送心跳包保持连接活跃
- **消息收发**: 支持文本、JSON、二进制数据的发送和接收
- **连接状态监控**: 实时监控连接状态变化
- **认证管理**: 支持用户认证和令牌管理
- **统计信息**: 提供详细的连接统计信息
- **类型安全**: 使用现代C++特性确保类型安全
- **异步优先**: 基于协程的异步设计

## 架构设计

```
SDKConnectionManager
├── 配置管理 (SDKConnectionConfig)
├── 连接状态管理 (ConnectionState/ConnectionEvent)
├── 消息收发 (MessageReceivedCallback)
├── 统计信息 (ConnectionStats)
└── 底层长连接 (LongConnectionClient)
    └── WebSocket客户端 (WSClient)
```

## 基本用法

### 1. 创建配置

```cpp
#include "im/sdk/src/connection/SDKConnectionManager.h"

using namespace roc::im::sdk;

// 创建连接配置
SDKConnectionConfig config;
config.set_server("localhost", "8080", "/im")
      .set_ssl(false)
      .set_auth("user123", "token456")
      .set_app_info("app001", "secret123")
      .set_connect_timeout(10000)
      .set_heartbeat(30000, 10000)
      .set_reconnect(true, 5, 2000)
      .set_protocol_version("1.0")
      .set_client_version("1.0.0");
```

### 2. 创建连接管理器

```cpp
boost::asio::io_context io_context;

// 创建连接管理器
auto connection_manager = std::make_shared<SDKConnectionManager>(config, io_context);
```

### 3. 设置回调函数

```cpp
// 设置消息接收回调
connection_manager->set_message_received_callback([](const void* data, size_t size) {
    std::string message(static_cast<const char*>(data), size);
    std::cout << "Received: " << message << std::endl;
});

// 设置连接状态回调
connection_manager->set_connection_state_callback([](ConnectionState state, ConnectionEvent event, const std::string& reason) {
    std::cout << "Connection state: " << static_cast<int>(state) 
              << ", event: " << static_cast<int>(event) 
              << ", reason: " << reason << std::endl;
});
```

### 4. 连接和消息发送

```cpp
boost::asio::co_spawn(io_context, [&]() -> boost::asio::awaitable<void> {
    // 连接到服务器
    auto connect_result = co_await connection_manager->connect();
    if (!connect_result) {
        std::cerr << "Connection failed: " << connect_result.error().to_string() << std::endl;
        co_return;
    }
    
    // 发送文本消息
    auto send_result = co_await connection_manager->send_message("Hello, Server!");
    if (send_result) {
        std::cout << "Message sent: " << send_result.value() << " bytes" << std::endl;
    }
    
    // 发送JSON消息
    std::string json_msg = R"({"type":"chat","content":"Hello from JSON!"})";
    auto json_result = co_await connection_manager->send_json_message(json_msg);
    if (json_result) {
        std::cout << "JSON message sent: " << json_result.value() << " bytes" << std::endl;
    }
    
    // 断开连接
    co_await connection_manager->disconnect();
}, boost::asio::detached);

// 运行IO上下文
io_context.run();
```

## 配置选项

### 服务器配置

| 方法 | 参数 | 说明 |
|------|------|------|
| `set_server()` | host, port, path | 设置服务器地址和路径 |
| `set_ssl()` | enable | 启用/禁用SSL |

### 认证配置

| 方法 | 参数 | 说明 |
|------|------|------|
| `set_auth()` | user_id, token | 设置用户认证信息 |
| `set_app_info()` | app_id, app_secret | 设置应用信息 |

### 连接配置

| 方法 | 参数 | 说明 |
|------|------|------|
| `set_connect_timeout()` | timeout_ms | 连接超时时间 |
| `set_heartbeat()` | interval_ms, timeout_ms | 心跳间隔和超时 |
| `set_reconnect()` | enable, max_attempts, backoff_ms | 重连配置 |

### 协议配置

| 方法 | 参数 | 说明 |
|------|------|------|
| `set_protocol_version()` | version | 协议版本 |
| `set_client_version()` | version | 客户端版本 |

## 连接状态

### ConnectionState 枚举

- `Disconnected`: 未连接
- `Connecting`: 连接中
- `Connected`: 已连接
- `Reconnecting`: 重连中
- `Disconnecting`: 断开连接中

### ConnectionEvent 枚举

- `Connected`: 连接成功
- `Disconnected`: 连接断开
- `Reconnecting`: 开始重连
- `ReconnectFailed`: 重连失败
- `AuthFailed`: 认证失败
- `Timeout`: 连接超时

## 消息发送

### 发送文本消息

```cpp
auto result = co_await connection_manager->send_message("Hello, World!");
```

### 发送二进制数据

```cpp
std::vector<uint8_t> data = {0x01, 0x02, 0x03};
auto result = co_await connection_manager->send_message(data.data(), data.size());
```

### 发送JSON消息

```cpp
std::string json = R"({"type":"message","content":"Hello"})";
auto result = co_await connection_manager->send_json_message(json);
```

## 统计信息

```cpp
auto stats = connection_manager->get_stats();
std::cout << "Messages sent: " << stats.messages_sent << std::endl;
std::cout << "Messages received: " << stats.messages_received << std::endl;
std::cout << "Bytes sent: " << stats.bytes_sent << std::endl;
std::cout << "Bytes received: " << stats.bytes_received << std::endl;
std::cout << "Reconnect count: " << stats.reconnect_count << std::endl;
```

## 错误处理

所有异步方法都返回 `std::expected<T, roc::error::Error>`，可以这样处理错误：

```cpp
auto result = co_await connection_manager->connect();
if (!result) {
    std::cerr << "Error: " << result.error().to_string() << std::endl;
    // 处理错误
} else {
    std::cout << "Success!" << std::endl;
    // 处理成功情况
}
```

## 线程安全

- 所有公共方法都是线程安全的
- 回调函数可能在不同线程中调用，需要注意线程安全
- 建议在单个IO线程中使用，避免跨线程调用

## 最佳实践

1. **配置管理**: 使用链式调用配置连接参数
2. **错误处理**: 总是检查异步操作的返回值
3. **资源管理**: 使用RAII和智能指针管理资源
4. **异步编程**: 使用协程避免阻塞操作
5. **状态监控**: 监听连接状态变化，及时处理断线重连
6. **统计监控**: 定期检查统计信息，监控连接质量

## 示例程序

参考 `examples/sdk_connection_example.cpp` 获取完整的使用示例。

## 依赖关系

- `base/network/include/LongConnectionClient.h`: 底层长连接客户端
- `im/base/error/Error.h`: 错误处理
- `boost/asio.hpp`: 异步IO
- `boost/json.hpp`: JSON处理

## 编译要求

- C++20 或更高版本
- Boost 1.75 或更高版本
- 支持协程的编译器 (GCC 10+, Clang 10+, MSVC 2019+) 