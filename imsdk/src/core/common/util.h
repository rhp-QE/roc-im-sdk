#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <memory>
#include <string>
#include <utility>
#include <stdexcept>
namespace roc::imsdk::core::util {

inline std::string key_for_user(const std::string& user_id, const std::string& key) {
    return "k" + user_id + "_" + key;
}

/**
 * Generate single chat conversation ID in format "0:1:smaller_uid:larger_uid"
 * @param user_id1 First user ID
 * @param user_id2 Second user ID
 * @return Conversation ID string
 */
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

/**
 * Parse two user IDs from single chat conversation ID
 * @param conv_id Conversation ID in format "0:1:user_id1:user_id2"
 * @return Pair of user IDs (first, second), empty strings if format is invalid
 */
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

} // namespace roc::imsdk::core