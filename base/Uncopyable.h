//
// Uncopyable.h
//
// 不可复制基类
// 继承此类的对象不能被复制，只能移动
//
// author: AI Assistant
// date: 2025-01-xx
//

#ifndef ROC_BASE_UNCOPYABLE_H
#define ROC_BASE_UNCOPYABLE_H

namespace roc::base {

class uncopyable {
protected:
    // 构造函数和析构函数
    uncopyable() = default;
    ~uncopyable() = default;

private:
    // 禁用拷贝构造函数和拷贝赋值运算符
    uncopyable(const uncopyable&) = delete;
    uncopyable& operator=(const uncopyable&) = delete;

    // 允许移动构造函数和移动赋值运算符
    uncopyable(uncopyable&&) = default;
    uncopyable& operator=(uncopyable&&) = default;
};

} // namespace roc::base

#endif // ROC_BASE_UNCOPYABLE_H 