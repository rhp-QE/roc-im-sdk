#pragma once

#include <string>
namespace roc::imsdk::core::util {

inline std::string key_for_user(const std::string& user_id, const std::string& key) {
    return user_id + "_" + key;
}

} // namespace roc::imsdk::core