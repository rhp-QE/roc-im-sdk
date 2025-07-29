//
// SerialQueue.h
//
// author: Ruan Huipeng
// date : 2025-03-21
// 

#ifndef ROC_THREADPOOL_SERIALQUEUE_H
#define ROC_THREADPOOL_SERIALQUEUE_H

namespace roc::threadpool {

class SerialQueue {
public:
    // 构造函数
    explicit SerialQueue(int value);

    // 拷贝构造函数
    SerialQueue(const SerialQueue& other);

    // 移动构造函数
    SerialQueue(SerialQueue&& other) noexcept;

    // 拷贝赋值操作符（使用拷贝-交换技术）
    SerialQueue& operator=(SerialQueue other);

    // 移动赋值操作符
    SerialQueue& operator=(SerialQueue&& other) noexcept;

    // 成员函数
    void setValue(int value);
    int getValue() const;

private:
    int value; // 成员变量
};

} // namespace roc::threadpool

#endif // ROC_THREADPOOL_SERIALQUEUE_H
