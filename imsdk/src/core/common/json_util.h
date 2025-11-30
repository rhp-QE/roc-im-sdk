#pragma once

#include "imsdk/base/include/network/Error.h"
#include <expected>
#include <string>
#include <unordered_map>
#include <vector>

namespace roc::imsdk::core::json_util {

/// JSON string -> std::unordered_map<std::string, std::string>
std::expected<std::unordered_map<std::string, std::string>, roc::error::Error> 
    MapParseFromString(const std::string& ext);

/// std::unordered_map<std::string, std::string> -> JSON string
std::expected<std::string, roc::error::Error> 
    MapSerializeAsString(const std::unordered_map<std::string, std::string>& ext_map);

/// JSON string -> std::vector<int32_t>
std::expected<std::vector<int32_t>, roc::error::Error> 
    Int32VectorParseFromString(const std::string& json_str);

/// std::vector<int32_t> -> JSON string
std::expected<std::string, roc::error::Error> 
    Int32VectorSerializeAsString(const std::vector<int32_t>& vec);

/// JSON string -> std::vector<std::string>
std::expected<std::vector<std::string>, roc::error::Error> 
    StringVectorParseFromString(const std::string& json_str);

/// std::vector<std::string> -> JSON string
std::expected<std::string, roc::error::Error> 
    StringVectorSerializeAsString(const std::vector<std::string>& vec);

} // namespace roc::imsdk::core::json_util

