#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/network/Error.h"
#include <boost/json.hpp>
#include <expected>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <stdexcept>
#include <chrono>
namespace roc::imsdk::core::util {

inline std::string key_for_user(const std::string& user_id, const std::string& key) {
    return "k" + user_id + "_" + key;
}

inline std::string generate_single_conv_id(const std::string& user_id1, const std::string& user_id2) {
    // Determine smaller and larger UIDs
    std::string smaller_uid, larger_uid;
    if (user_id1 < user_id2) {
        smaller_uid = user_id1;
        larger_uid = user_id2;
    } else {
        smaller_uid = user_id2;
        larger_uid = user_id1;
    }
    
    // Construct conv_id in format "0:1:smaller_uid:larger_uid"
    return "0:1:" + smaller_uid + ":" + larger_uid;
}

inline std::pair<std::string, std::string> parse_single_conv_id(const std::string& conv_id) {
    // Check if conv_id starts with "0:1:"
    if (conv_id.length() < 4 || conv_id.substr(0, 4) != "0:1:") {
        return std::make_pair("", "");
    }
    
    // Find the third colon (after "0:1:")
    size_t third_colon = conv_id.find(':', 4);
    if (third_colon == std::string::npos) {
        return std::make_pair("", "");
    }
    
    // Extract user IDs
    std::string user_id1 = conv_id.substr(4, third_colon - 4);
    std::string user_id2 = conv_id.substr(third_colon + 1);
    
    // Check if user IDs are not empty
    if (user_id1.empty() || user_id2.empty()) {
        return std::make_pair("", "");
    }
    
    return std::make_pair(user_id1, user_id2);
}

inline bool message_send_from_me(std::string login_uid, std::string from_uid) {
    return login_uid == from_uid;
}

inline double current_time_since1970() {
    // 获取当前时间点（UTC）
    auto now = std::chrono::system_clock::now();
    
    // 转换为自 1970-01-01 00:00:00 UTC 以来的持续时间
    auto duration = now.time_since_epoch();
    
    // 转换为秒（double 类型，包含小数微秒/纳秒）
    return std::chrono::duration<double>(duration).count();
}

inline std::expected<std::unordered_map<std::string, std::string>, roc::error::Error> MapParseFromString(const std::string& ext) {
    std::unordered_map<std::string, std::string> result;
    if (ext.empty()) {
        return result;
    }

    try {
        auto json_value = boost::json::parse(ext);
        if (!json_value.is_object()) {
            return std::unexpected(roc::error::make_error(3006, "JSON is not an object"));
        }

        const auto& obj = json_value.as_object();
        for (const auto& item : obj) {
            if (item.value().is_string()) {
                result.emplace(item.key_c_str(), std::string(item.value().as_string()));
            } else {
                result.emplace(item.key_c_str(), boost::json::serialize(item.value()));
            }
        }
        return result;
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3006, "Failed to parse ext string", e.what()));
    }
}

inline std::expected<std::string, roc::error::Error> MapSerializeAsString(const std::unordered_map<std::string, std::string>& ext_map) {
    try {
        boost::json::object json_obj;
        for (const auto& [key, value] : ext_map) {
            json_obj[key] = value;
        }
        return boost::json::serialize(json_obj);
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3005, "Failed to serialize ext map", e.what()));
    }
}

} // namespace roc::imsdk::core::util