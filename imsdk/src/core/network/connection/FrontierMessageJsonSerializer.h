//
// FrontierMessageJsonSerializer.h
//
// FrionterMessage JSON 序列化和反序列化工具类
//
// author: Ruan Huipeng
// date: 2025-12-07
//

#pragma once

#include <boost/json/value.hpp>
#include <expected>
#include <string>
#include <unordered_map>
#include "imsdk/base/include/network/Error.h"

namespace roc::imsdk::network {

// 前置声明
struct FrionterMessage;

/// FrionterMessage JSON 序列化和反序列化工具类
class FrontierMessageJsonSerializer {
public:
    static std::string ToJsonString(const FrionterMessage& msg);

    static boost::json::value ToJson(const FrionterMessage& msg);

    static std::expected<FrionterMessage, roc::error::Error> FromJsonString(const std::string& json_str);

    static std::expected<FrionterMessage, roc::error::Error> FromJsonBuffer(const void* data, size_t size);

    static std::expected<FrionterMessage, roc::error::Error> FromJson(const boost::json::value& json_value);

private:
    static boost::json::object MetadataToJsonObject(const std::unordered_map<std::string, std::string>& metadata);

    static std::unordered_map<std::string, std::string> JsonObjectToMetadata(const boost::json::object& obj);

    // 辅助方法：将 payload 字节数组转换为 base64 编码的 JSON 字符串
    static std::string PayloadToBase64String(const std::vector<uint8_t>& payload);

    // 辅助方法：从 base64 编码的 JSON 字符串转换为 payload 字节数组
    static std::expected<std::vector<uint8_t>, roc::error::Error> Base64StringToPayload(const std::string& base64_str);
};

} // namespace roc::imsdk::network


