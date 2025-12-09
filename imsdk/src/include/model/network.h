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
struct FrionterMessage {
    std::string request_id;                    // 请求ID（用于请求-响应匹配）
    std::string type;                          // 消息类型
    std::string service;                       // 目标服务名称
    std::string method;                        // 服务方法名称
    std::vector<uint8_t> payload;              // 消息有效载荷/数据（字节数组，业务数据）
    std::string error;                         // 错误信息（响应时使用）
    int64_t timestamp;                         // 时间戳
    std::unordered_map<std::string, std::string> metadata;  // 元数据（用于传递验证信息如token、track_id等）
};

} // namespace roc::imsdk::network