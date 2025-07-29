// ThreadPool.cc
//
// author: Ruan Huipeng
// date : 2025-03-21
//

#include "ThreadPool.h"

// 使用命名空间
namespace roc::threadpool {

// 显式构造函数实现
ThreadPool::ThreadPool(int value) : value(value) {}

// 拷贝构造函数实现
ThreadPool::ThreadPool(const ThreadPool& other) : value(other.value) {}

// 移动构造函数实现
ThreadPool::ThreadPool(ThreadPool&& other) noexcept : value(other.value) {
    other.value = 0; // 将其他对象的值置为默认值
}

// 拷贝赋值操作符实现（拷贝-交换技术）
ThreadPool& ThreadPool::operator=(ThreadPool other) {
    value = other.value; // 交换值
    return *this; // 返回当前对象
}

// 移动赋值操作符实现
ThreadPool& ThreadPool::operator=(ThreadPool&& other) noexcept {
    if (this == &other) {
        return *this; // 自我赋值检查
    }

    value = other.value; // 移动值
    other.value = 0; // 将其他对象的值置为默认值
    return *this;
}

// 设置值的函数实现
void ThreadPool::setValue(int value) {
    this->value = value;
}

// 获取值的函数实现
int ThreadPool::getValue() const {
    return value;
}

} // namespace roc::threadpool
