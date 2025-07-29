# LongConnectionClient 使用指南

## 概述

`LongConnectionClient` 是一个基于 `IWSClient` 接口设计的长连接客户端，内部使用 `WSClient` 实现，但通过接口调用方法。提供了以下功能：

- **自动心跳检测**：定期发送心跳消息，检测连接状态
- **自动重连机制**：连接断开时自动尝试重连
- **数据接收回调**：注册回调函数处理接收到的数据
- **连接状态监控**：监控连接状态变化并通知用户
- **异步数据发送**：支持异步发送二进制和字符串数据

## 主要特性

### 1. 接口化设计
- 基于 `IWSClient` 接口设计，内部使用 `WSClient` 实现
- 通过接口调用 WebSocket 方法，便于将来替换底层实现
- 提高代码复用性和可维护性
- 支持自定义 WebSocket 客户端，只需实现 `IWSClient` 接口

### 2. 心跳机制
- 使用 WebSocket 原生 ping/pong 机制
- 可配置心跳间隔和超时时间
- 自动检测连接状态
- 心跳超时时触发重连

### 3. 自动重连
- 支持配置最大重连次数
- 可配置重连退避时间
- 智能重连策略

### 4. 回调机制
- 数据接收回调
- 连接状态变化回调
- 线程安全的回调处理

### 5. 配置灵活
- 继承自 `WSClientConfig`
- 支持链式配置
- 丰富的配置选项

## 快速开始

### 1. 基本使用

```cpp
#include "im/base/lc/impl/ws/LongConnectionClient.h"

using namespace roc::base::net;

// 创建配置
LongConnectionConfig config("localhost", "8080");
config.set_path("/ws")
      .set_heartbeat_interval(30000)      // 30秒心跳
      .set_auto_reconnect(true);          // 启用自动重连

// 创建客户端
boost::asio::io_context io_context;
auto client = std::make_shared<LongConnectionClient>(config, io_context);

// 注册回调
client->set_data_received_callback([](const void* data, size_t size) {
    std::string message(static_cast<const char*>(data), size);
    std::cout << "Received: " << message << std::endl;
});

client->set_connection_status_callback([](bool connected, const std::string& reason) {
    std::cout << (connected ? "Connected" : "Disconnected") << ": " << reason << std::endl;
});

// 连接
auto result = co_await client->connect();
if (result) {
    // 发送数据
    co_await client->send_message("Hello, server!");
}
```

### 2. 配置选项

```cpp
LongConnectionConfig config("example.com", "443");
config
    // WebSocket 基本配置
    .set_path("/api/ws")
    .add_header("Authorization", "Bearer token123")
    .add_query_param("client_id", "my_client")
    
    // 心跳配置
    .set_heartbeat_interval(30000)        // 30秒心跳间隔
    .set_heartbeat_timeout(10000)         // 10秒心跳超时
    .set_heartbeat_payload("client_id")   // ping 帧负载数据
    
    // 重连配置
    .set_auto_reconnect(true)             // 启用自动重连
    .set_max_reconnect_attempts(5)        // 最大重连5次
    .set_reconnect_backoff(1000);         // 1秒重连退避
```

### 3. 数据发送

```cpp
// 发送字符串
auto result = co_await client->send_message("Hello, world!");

// 发送二进制数据
std::vector<uint8_t> binary_data = {0x01, 0x02, 0x03, 0x04};
auto result = co_await client->send_data(binary_data.data(), binary_data.size());

if (result) {
    std::cout << "Sent " << result.value() << " bytes" << std::endl;
} else {
    std::cout << "Send failed: " << result.error() << std::endl;
}
```

### 4. 连接管理

```cpp
// 检查连接状态
if (client->is_connected()) {
    std::cout << "Client is connected" << std::endl;
}

// 断开连接
auto result = co_await client->disconnect();
if (result) {
    std::cout << "Disconnected successfully" << std::endl;
}
```

## 配置详解

### LongConnectionConfig

继承自 `WSClientConfig`，增加了以下配置选项：

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| `heartbeat_interval_` | uint32_t | 30000 | 心跳间隔（毫秒） |
| `heartbeat_timeout_` | uint32_t | 10000 | 心跳超时时间（毫秒） |
| `heartbeat_payload_` | string | "ping" | ping 帧负载数据 |
| `enable_auto_reconnect_` | bool | true | 是否启用自动重连 |
| `max_reconnect_attempts_` | uint32_t | 5 | 最大重连次数 |
| `reconnect_backoff_ms_` | uint32_t | 1000 | 重连退避时间（毫秒） |

### 配置方法

所有配置方法都支持链式调用：

```cpp
LongConnectionConfig config("localhost", "8080");
config
    .set_heartbeat_interval(30000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("client_id") // ping 帧负载数据
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000);
```

## 回调函数

### 数据接收回调

```cpp
using DataReceivedCallback = std::function<void(const void* data, size_t size)>;

client->set_data_received_callback([](const void* data, size_t size) {
    // 处理接收到的数据
    std::string message(static_cast<const char*>(data), size);
    std::cout << "Received: " << message << std::endl;
});
```

### 连接状态回调

```cpp
using ConnectionStatusCallback = std::function<void(bool connected, const std::string& reason)>;

client->set_connection_status_callback([](bool connected, const std::string& reason) {
    if (connected) {
        std::cout << "Connected: " << reason << std::endl;
    } else {
        std::cout << "Disconnected: " << reason << std::endl;
    }
});
```

## 心跳机制

### 工作原理

1. 客户端定期发送心跳消息（默认30秒间隔）
2. 如果心跳超时（默认10秒），认为连接断开
3. 触发自动重连机制

### WebSocket ping/pong 机制

- 使用 WebSocket 协议标准的 ping/pong 控制帧
- 更高效，网络开销更小
- 服务器会自动响应 pong 帧
- 支持在 ping 帧中携带负载数据（最多125字节）

### 心跳配置

```cpp
config
    .set_heartbeat_interval(30000)    // 30秒发送一次 ping
    .set_heartbeat_timeout(10000)     // 10秒内没收到 pong 认为超时
    .set_heartbeat_payload("client_id"); // ping 帧负载数据
```

## 自动重连

### 重连策略

1. 连接断开时自动启动重连
2. 使用指数退避策略（可配置）
3. 达到最大重连次数后停止
4. 重连成功后重置重连计数

### 重连配置

```cpp
config
    .set_auto_reconnect(true)         // 启用自动重连
    .set_max_reconnect_attempts(5)    // 最多重连5次
    .set_reconnect_backoff(1000);     // 每次重连间隔1秒
```

## 错误处理

### 常见错误

- **连接失败**：检查服务器地址和端口
- **心跳超时**：检查网络连接和服务器状态
- **重连失败**：检查网络环境和服务器可用性

### 错误处理示例

```cpp
auto result = co_await client->connect();
if (!result) {
    std::cout << "Connection failed: " << result.error() << std::endl;
    // 处理连接失败
    return;
}

auto send_result = co_await client->send_message("test");
if (!send_result) {
    std::cout << "Send failed: " << send_result.error() << std::endl;
    // 处理发送失败
}
```

## 线程安全

- 所有公共方法都是线程安全的
- 回调函数在 IO 线程中执行
- 内部状态使用原子变量保护

## 性能考虑

- 使用异步 I/O，不会阻塞线程
- 心跳间隔不宜过短（建议 >= 10秒）
- 重连退避时间建议递增（1秒、2秒、4秒等）

## 完整示例

参考 `LongConnectionClientExample.cpp` 文件，其中包含了完整的使用示例，包括：

- 基本连接和数据发送
- 心跳机制演示
- 自动重连功能演示
- 回调函数注册和使用

## WebSocket ping/pong 机制

### 优势

1. **协议标准**：使用 WebSocket RFC 6455 标准定义的 ping/pong 控制帧
2. **高效性**：控制帧比数据帧更轻量，网络开销更小
3. **自动响应**：大多数 WebSocket 服务器会自动响应 ping 帧
4. **兼容性**：与所有符合标准的 WebSocket 服务器兼容

### 工作原理

1. 客户端发送 ping 控制帧（可携带负载数据）
2. 服务器自动响应 pong 控制帧
3. 客户端检测到 pong 响应后更新心跳时间
4. 如果超时未收到响应，触发重连

### 配置示例

```cpp
LongConnectionConfig config("example.com", "443");
config
    .set_heartbeat_interval(30000)      // 30秒发送一次 ping
    .set_heartbeat_timeout(10000)       // 10秒内没收到 pong 认为超时
    .set_heartbeat_payload("client_id"); // ping 帧的负载数据
```



## 注意事项

1. **IO 上下文**：确保 IO 上下文在客户端生命周期内有效
2. **资源管理**：及时调用 `disconnect()` 释放资源
3. **回调函数**：回调函数中避免执行耗时操作
4. **配置验证**：确保配置参数合理（心跳间隔 > 心跳超时）
5. **异常处理**：妥善处理异步操作的异常
6. **ping/pong 负载**：ping 帧负载限制为 125 字节
7. **服务器兼容性**：确保服务器支持 ping/pong 控制帧 