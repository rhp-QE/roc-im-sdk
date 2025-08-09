#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread> // Added for std::this_thread::get_id()
#include <functional> // Added for std::hash

namespace roc::base::util {

// 检查callback 是否为空 并调用, callback 接收多个参数, 如果有返回值 则返回返回值
// template <typename... Args>
// auto invoke(std::function<void(Args...)>& callback, Args&&... args) -> decltype(callback(args...)) {
//     if (callback) {
//         return callback(std::forward<Args>(args)...);
//     } else {
//         return decltype(callback(std::forward<Args>(args)...))();
//     }
// }

template<typename Re, typename... Args, typename... FuncArgs>
Re safe_invoke_block(const std::function<Re(FuncArgs...)>& func, Args&&... args) {
    if (func) {
        return func(std::forward<Args>(args)...);
    } else if constexpr (!std::is_void_v<Re>) {
        return Re();
    }
}

template<typename ValueType, typename Func>
 std::unordered_map<std::string, std::vector<ValueType>> 
 group_by_key(const std::vector<ValueType>& vec, Func &&key_for_value) 
 {
     std::unordered_map<std::string, std::vector<ValueType>> map;
     for (const auto& value : vec) {
         std::string key = key_for_value(value);
         map[key].push_back(value);
     }
     return map;
 }

template <typename InputType, typename Transformer>
auto transform(
    const std::vector<InputType>& input,
    Transformer&& transformer)
{
    using OutputType = std::decay_t<decltype(transformer(*input.begin()))>;
    std::vector<OutputType> output;
    output.reserve(input.size());
    std::transform(
        input.begin(), 
        input.end(),
        std::back_inserter(output),
        std::forward<Transformer>(transformer));
    return output;
}

/// 生成唯一客户端消息ID
/// 返回24位字符串：时间戳(10位) + 纳秒(6位) + 随机字母(8位)
inline std::string uuid() {
    // 使用时间戳、纳秒和随机数生成唯一客户端消息ID
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // 获取纳秒级精度
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    int64_t ns = nanoseconds.count() % 1000000; // 取模确保6位数字
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 生成8位大小写英文字母的随机字符串
    std::string random_str;
    random_str.reserve(8);
    std::uniform_int_distribution<> letter_dis(0, 51); // 0-25为小写字母a-z，26-51为大写字母A-Z
    
    for (int i = 0; i < 8; ++i) {
        int letter_index = letter_dis(gen);
        if (letter_index < 26) {
            random_str += 'a' + letter_index; // 小写字母
        } else {
            random_str += 'A' + (letter_index - 26); // 大写字母
        }
    }
    
    // 生成ID：时间戳(10位) + 纳秒(6位) + 随机字母(8位) = 24位
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(10) << time_t  // 10位时间戳
       << std::setfill('0') << std::setw(6) << ns        // 6位纳秒
       << random_str;  // 8位随机字母
    
    return ss.str();
}

} // namespace roc::base::utils