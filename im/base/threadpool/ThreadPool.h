//
// ThreadPool.h
//
// author: Ruan Huipeng
// date : 2025-03-21
// 

#ifndef ROC_THREADPOOL_THREADPOOL_H
#define ROC_THREADPOOL_THREADPOOL_H

namespace roc::threadpool {

class ThreadPool {
public:
    // 构造函数
    explicit ThreadPool(int value);

    // 拷贝构造函数
    ThreadPool(const ThreadPool& other);

    // 移动构造函数
    ThreadPool(ThreadPool&& other) noexcept;

    // 拷贝赋值操作符（使用拷贝-交换技术）
    ThreadPool& operator=(ThreadPool other);

    // 移动赋值操作符
    ThreadPool& operator=(ThreadPool&& other) noexcept;

    // 成员函数
    void setValue(int value);
    int getValue() const;

private:
    int value; // 成员变量
};

} // namespace roc::threadpool

#endif // ROC_THREADPOOL_THREADPOOL_H
