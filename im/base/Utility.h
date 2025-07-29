//
// Utility.h
//
// author: Ruan Huipeng
// date : 2025-03-02
// 

#ifndef ROC_BASE_UTIL_UTILITY_H
#define ROC_BASE_UTIL_UTILITY_H

#include "im/base/noncopyable.h"
#include <boost/asio/buffer.hpp>
#include <concepts>
#include <functional>

namespace roc::base::util {

template<typename Re, typename... FuncArgs, typename... Args>
Re safe_invoke_block(const std::function<Re(FuncArgs...)>& func, Args&&... args) {
    if (func) {
        return func(std::forward<Args>(args)...);
    } 
}

// ------------------------
template <std::invocable F>
class defer : noncopyable {
    F func_;
public:
    explicit defer(F func) : func_(func) {}
    ~defer<F>() { std::invoke(func_); }
};
// ------------------------


inline uint32_t decode_uint32_LE(char number_bytes[]) {
    uint32_t number = 0;
    for (int i = 0; i < 4; ++i ) {
        number |= static_cast<uint8_t>(number_bytes[i]) << (i<<3);
    }
    return number;
}

inline void encode_uint32_LE(uint32_t number, char number_bytes[]) {
    for (int i = 0; i < 4; ++i) {
        number_bytes[i] = static_cast<char>(number & 0xFF);
        number >>= 8;
    }
}

inline void copy(char* from, size_t size, std::vector<boost::asio::mutable_buffer> to) {
    size_t written = 0;
    for (const auto& buf : to) {
        if (size == written) {
            break;
        }
        const size_t copy_size = std::min(buf.size(), size - written);
        std::memcpy(buf.data(), from + written, copy_size);
        written += copy_size;
    }
}

} // namespace roc::base::util

#endif // ROC_BASE_UTIL_UTILITY_H
