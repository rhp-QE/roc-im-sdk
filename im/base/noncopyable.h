#ifndef ROC_NONCOPYABLE_H
#define ROC_NONCOPYABLE_H

namespace roc {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator= (const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}// namespace noncopyable

#endif