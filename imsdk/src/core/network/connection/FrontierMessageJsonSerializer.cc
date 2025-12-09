//
// FrontierMessageJsonSerializer.cc
//
// FrionterMessage JSON 序列化和反序列化工具类实现
//
// author: Ruan Huipeng
// date: 2025-12-07
//

#include "FrontierMessageJsonSerializer.h"
#include "imsdk/src/include/model/network.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/core/common/logger_macro.h"
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/object.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <algorithm>
#include <stdexcept>

namespace roc::imsdk::network {

std::string FrontierMessageJsonSerializer::ToJsonString(const FrionterMessage& msg) {
    boost::json::value json_value = ToJson(msg);
    return boost::json::serialize(json_value);
}

boost::json::value FrontierMessageJsonSerializer::ToJson(const FrionterMessage& msg) {
    boost::json::object obj;
    
    obj["requestID"] = msg.request_id;
    obj["type"] = msg.type;
    obj["service"] = msg.service;
    obj["method"] = msg.method;
    obj["payload"] = PayloadToBase64String(msg.payload);
    obj["error"] = msg.error;
    obj["timestamp"] = msg.timestamp;
    obj["metadata"] = MetadataToJsonObject(msg.metadata);
    
    return obj;
}

std::expected<FrionterMessage, roc::error::Error> FrontierMessageJsonSerializer::FromJsonString(const std::string& json_str) {
    return FromJsonBuffer(json_str.data(), json_str.size());
}

std::expected<FrionterMessage, roc::error::Error> FrontierMessageJsonSerializer::FromJsonBuffer(const void* data, size_t size) {
    try {
        if (!data || size == 0) {
            return std::unexpected(roc::error::make_error(1001, "Invalid buffer: data is null or size is zero"));
        }
        
        const char* char_data = static_cast<const char*>(data);
        boost::json::string_view sv(char_data, size);
        boost::json::value json_value = boost::json::parse(sv);
        return FromJson(json_value);
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(1002, "Failed to parse JSON: " + std::string(e.what())));
    }
}

std::expected<FrionterMessage, roc::error::Error> FrontierMessageJsonSerializer::FromJson(const boost::json::value& json_value) {
    try {
        if (!json_value.is_object()) {
            return std::unexpected(roc::error::make_error(1002, "JSON value is not an object"));
        }

        const boost::json::object& obj = json_value.as_object();
        FrionterMessage msg;

        // requestID
        if (obj.contains("requestID") && obj.at("requestID").is_string()) {
            msg.request_id = std::string(obj.at("requestID").as_string());
        }

        // type
        if (obj.contains("type") && obj.at("type").is_string()) {
            msg.type = std::string(obj.at("type").as_string());
        } else {
            return std::unexpected(roc::error::make_error(1003, "Missing required field: type"));
        }

        // service
        if (obj.contains("service") && obj.at("service").is_string()) {
            msg.service = std::string(obj.at("service").as_string());
        }

        // method
        if (obj.contains("method") && obj.at("method").is_string()) {
            msg.method = std::string(obj.at("method").as_string());
        }

        // payload
        if (obj.contains("payload")) {
            if (obj.at("payload").is_string()) {
                auto payload_result = Base64StringToPayload(std::string(obj.at("payload").as_string()));
                if (!payload_result) {
                    return std::unexpected(payload_result.error());
                }
                msg.payload = std::move(payload_result.value());
            } else if (obj.at("payload").is_null()) {
                msg.payload = {};
            }
        }

        // error
        if (obj.contains("error") && obj.at("error").is_string()) {
            msg.error = std::string(obj.at("error").as_string());
        }

        // timestamp
        if (obj.contains("timestamp")) {
            if (obj.at("timestamp").is_int64()) {
                msg.timestamp = obj.at("timestamp").as_int64();
            } else if (obj.at("timestamp").is_uint64()) {
                msg.timestamp = static_cast<int64_t>(obj.at("timestamp").as_uint64());
            } else {
                return std::unexpected(roc::error::make_error(1004, "Invalid timestamp format"));
            }
        }

        // metadata
        if (obj.contains("metadata") && obj.at("metadata").is_object()) {
            msg.metadata = JsonObjectToMetadata(obj.at("metadata").as_object());
        }

        return msg;
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(1005, "Failed to deserialize JSON: " + std::string(e.what())));
    }
}

boost::json::object FrontierMessageJsonSerializer::MetadataToJsonObject(const std::unordered_map<std::string, std::string>& metadata) {
    boost::json::object obj;
    for (const auto& [key, value] : metadata) {
        obj[key] = value;
    }
    return obj;
}

std::unordered_map<std::string, std::string> FrontierMessageJsonSerializer::JsonObjectToMetadata(const boost::json::object& obj) {
    std::unordered_map<std::string, std::string> metadata;
    for (const auto& [key, value] : obj) {
        if (value.is_string()) {
            metadata[std::string(key)] = std::string(value.as_string());
        }
    }
    return metadata;
}

std::string FrontierMessageJsonSerializer::PayloadToBase64String(const std::vector<uint8_t>& payload) {
    if (payload.empty()) {
        return "";
    }
    
    // 使用 base64 编码将二进制数据转换为字符串
    std::string encoded;
    encoded.resize(boost::beast::detail::base64::encoded_size(payload.size()));
    auto n = boost::beast::detail::base64::encode(encoded.data(), payload.data(), payload.size());
    encoded.resize(n);
    return encoded;
}

std::expected<std::vector<uint8_t>, roc::error::Error> FrontierMessageJsonSerializer::Base64StringToPayload(const std::string& base64_str) {
    if (base64_str.empty()) {
        return std::vector<uint8_t>();
    }

    try {
        // 从 base64 解码
        std::vector<uint8_t> decoded;
        decoded.resize(boost::beast::detail::base64::decoded_size(base64_str.size()));
        auto [bytes_written, chars_read] = boost::beast::detail::base64::decode(decoded.data(), base64_str.data(), base64_str.size());
        
        if (chars_read != base64_str.size()) {
            return std::unexpected(roc::error::make_error(1006, "Failed to decode base64 payload: incomplete decode"));
        }
        
        decoded.resize(bytes_written);
        return decoded;
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(1007, "Failed to decode payload: " + std::string(e.what())));
    }
}

} // namespace roc::imsdk::network

