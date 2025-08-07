#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>

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


// #include <type_traits>

// // 修改后的 invoke 函数模板
// template <typename... FuncArgs, typename... Args>
// auto invoke_1(std::function<void(FuncArgs...)>& callback, Args&&... args) 
//     -> std::enable_if_t<std::is_convertible_v<std::tuple<Args...>, std::tuple<FuncArgs...>>, void>
// {
//     if (callback) {
//         callback(std::forward<Args>(args)...);
//     }
//     // 对于 void 返回类型，无需返回默认值
// }

} // namespace roc::base::utils