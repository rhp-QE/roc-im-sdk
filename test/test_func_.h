#pragma once

#include <iostream>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

// 全局状态容器（线程安全）
class GlobalState {
private:
    std::mutex mtx;
    std::unordered_map<std::string, int> data;
    
public:
    // 纯函数：生成状态更新操作
    using StateUpdate = std::function<void(std::unordered_map<std::string, int>&)>;
    
    // 应用更新（有副作用，但被隔离）
    void applyUpdate(StateUpdate update) {
        std::lock_guard<std::mutex> lock(mtx);
        update(data);
    }
    
    // 获取当前状态快照
    std::unordered_map<std::string, int> getSnapshot() {
        std::lock_guard<std::mutex> lock(mtx);
        return data;
    }
};

// 纯函数业务逻辑
namespace BusinessLogic {
    // 纯函数：处理输入并生成状态更新
    using UpdateGenerator = std::function<GlobalState::StateUpdate(int input)>;
    
    static UpdateGenerator processValue(const std::string& key) {
        return [=](int input) {
            // 返回一个状态更新函数（闭包）
            return [=](std::unordered_map<std::string, int>& state) {
                // 实际更新操作
                state[key] = input * 2;
            };
        };
    }
    
    // 纯函数：复杂计算
    static int complexCalculation(int a, int b) {
        return a * b + (a + b);
    }
}

int test_func_() {
    GlobalState globalState;
    
    // 纯函数处理流程
    auto input = 5;
    
    // 阶段1：生成更新操作（纯函数）
    auto updateOp1 = BusinessLogic::processValue("resultA")(input);
    
    // 阶段2：另一个纯计算
    int intermediate = BusinessLogic::complexCalculation(input, 3);
    auto updateOp2 = BusinessLogic::processValue("resultB")(intermediate);
    
    // 阶段3：组合更新操作（纯函数）
    auto combinedUpdate = [=](std::unordered_map<std::string, int>& state) {
        updateOp1(state);
        updateOp2(state);
    };
    
    // 应用更新到全局状态（唯一副作用点）
    globalState.applyUpdate(combinedUpdate);
    
    // 获取最终结果
    auto finalState = globalState.getSnapshot();
    std::cout << "Result A: " << finalState["resultA"] << std::endl;
    std::cout << "Result B: " << finalState["resultB"] << std::endl;
    
    return 0;
}