#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace roc::imsdk::network {

enum class NetworkStatus {
    NETWORK_STATUS_UNKNOWN = 0,
    NETWORK_STATUS_CONNECTED = 1,
    NETWORK_STATUS_DISCONNECTED = 2,
};

// Message WebSocket消息结构（网关模式）
// 用于请求-响应匹配和消息路由
// 对标 Go 语言的 Message 结构体
struct FrontierMessage {
    std::string request_id;                    // 请求ID（用于请求-响应匹配），JSON字段名：requestID
    std::string type;                          // 消息类型，JSON字段名：type (request / response / push)
    std::string service;                       // 目标服务名称，JSON字段名：service
    std::string method;                        // 服务方法名称，JSON字段名：method
    std::vector<uint8_t> payload;              // 消息有效载荷/数据（字节数组，业务数据），JSON字段名：payload（base64编码）
    std::string error;                         // 错误信息（响应时使用），JSON字段名：error
    int64_t timestamp;                         // 时间戳，JSON字段名：timestamp
    std::unordered_map<std::string, std::string> metadata;  // 元数据（用于传递验证信息如token、track_id等），JSON字段名：metadata
};

// 消息类型
namespace FrontierMessageType {
    constexpr const char* Request   = "request";   // 请求消息（RPC调用）
    constexpr const char* Response  = "response";  // 响应消息
    constexpr const char* Auth      = "auth";      // 认证消息
    constexpr const char* Heartbeat = "heartbeat"; // 心跳消息
    constexpr const char* Push      = "push";      // 服务端主动推送消息
    constexpr const char* Error     = "error";     // 错误消息
}

} // namespace roc::imsdk::network