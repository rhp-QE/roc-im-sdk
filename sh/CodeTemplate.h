//
// CodeTemplate.h
//
// author: Ruan Huipeng
// date : current_time
// 

#ifndef CODETEMPLATEHH_H
#define CODETEMPLATEHH_H

namespace codenamespace {

class CodeTemplate {
public:
    // 构造函数
    explicit CodeTemplate(int value);

    // 拷贝构造函数
    CodeTemplate(const CodeTemplate& other);

    // 移动构造函数
    CodeTemplate(CodeTemplate&& other) noexcept;

    // 拷贝赋值操作符（使用拷贝-交换技术）
    CodeTemplate& operator=(CodeTemplate other);

    // 移动赋值操作符
    CodeTemplate& operator=(CodeTemplate&& other) noexcept;

    // 成员函数
    void setValue(int value);
    int getValue() const;

private:
    int value; // 成员变量
};

} // namespace codenamespace

#endif // CODETEMPLATEHH_H
