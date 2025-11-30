#include "json_util.h"

#include <boost/json.hpp>
#include <cstdint>

namespace roc::imsdk::core::json_util {

std::expected<std::unordered_map<std::string, std::string>, roc::error::Error> 
MapParseFromString(const std::string& ext) {
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

std::expected<std::string, roc::error::Error> 
MapSerializeAsString(const std::unordered_map<std::string, std::string>& ext_map) {
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

std::expected<std::vector<int32_t>, roc::error::Error> 
Int32VectorParseFromString(const std::string& json_str) {
    std::vector<int32_t> result;
    if (json_str.empty()) {
        return result;
    }

    try {
        auto json_value = boost::json::parse(json_str);
        if (!json_value.is_array()) {
            return std::unexpected(roc::error::make_error(3007, "JSON is not an array"));
        }

        const auto& arr = json_value.as_array();
        result.reserve(arr.size());
        for (const auto& item : arr) {
            if (item.is_int64()) {
                result.push_back(static_cast<int32_t>(item.as_int64()));
            } else if (item.is_number()) {
                result.push_back(static_cast<int32_t>(item.to_number<double>()));
            }
        }
        return result;
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3007, "Failed to parse int32 vector string", e.what()));
    }
}

std::expected<std::string, roc::error::Error> 
Int32VectorSerializeAsString(const std::vector<int32_t>& vec) {
    try {
        boost::json::array json_arr;
        for (const auto& val : vec) {
            json_arr.push_back(val);
        }
        return boost::json::serialize(json_arr);
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3008, "Failed to serialize int32 vector", e.what()));
    }
}

std::expected<std::vector<std::string>, roc::error::Error> 
StringVectorParseFromString(const std::string& json_str) {
    std::vector<std::string> result;
    if (json_str.empty()) {
        return result;
    }

    try {
        auto json_value = boost::json::parse(json_str);
        if (!json_value.is_array()) {
            return std::unexpected(roc::error::make_error(3009, "JSON is not an array"));
        }

        const auto& arr = json_value.as_array();
        result.reserve(arr.size());
        for (const auto& item : arr) {
            if (item.is_string()) {
                result.emplace_back(std::string(item.as_string()));
            }
        }
        return result;
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3009, "Failed to parse string vector string", e.what()));
    }
}

std::expected<std::string, roc::error::Error> 
StringVectorSerializeAsString(const std::vector<std::string>& vec) {
    try {
        boost::json::array json_arr;
        for (const auto& val : vec) {
            json_arr.push_back(boost::json::string(val));
        }
        return boost::json::serialize(json_arr);
    } catch (const std::exception& e) {
        return std::unexpected(roc::error::make_error(3010, "Failed to serialize string vector", e.what()));
    }
}

} // namespace roc::imsdk::core::json_util

