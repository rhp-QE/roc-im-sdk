//
// ParallelQueue.h
//
// author: Ruan Huipeng
// date : 2025-03-21
// 

#ifndef ROC_THREADPOOL_PARALLELQUEUE_H
#define ROC_THREADPOOL_PARALLELQUEUE_H

namespace roc::threadpool {

class ParallelQueue {
public:
    // 构造函数
    explicit ParallelQueue(int value);

    // 拷贝构造函数
    ParallelQueue(const ParallelQueue& other);

    // 移动构造函数
    ParallelQueue(ParallelQueue&& other) noexcept;

    // 拷贝赋值操作符（使用拷贝-交换技术）
    ParallelQueue& operator=(ParallelQueue other);

    // 移动赋值操作符
    ParallelQueue& operator=(ParallelQueue&& other) noexcept;

    // 成员函数
    void setValue(int value);
    int getValue() const;

private:
    int value; // 成员变量
};

} // namespace roc::threadpool

#endif // ROC_THREADPOOL_PARALLELQUEUE_H
