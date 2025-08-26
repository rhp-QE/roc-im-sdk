//
// ThreadSafeVector.h
//
// 线程安全的 std::vector 封装
// 提供完整的线程安全操作接口，支持读多写少的场景
//
// author: AI Assistant
// date: 2025-01-xx
// version: 2.0 - 根据代码审查意见改进
//

#pragma once

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace roc::base::containers {

template<typename T, typename Allocator = std::allocator<T>>
class ThreadSafeVector {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = typename std::vector<T, Allocator>::size_type;
    using difference_type = typename std::vector<T, Allocator>::difference_type;
    using reference = typename std::vector<T, Allocator>::reference;
    using const_reference = typename std::vector<T, Allocator>::const_reference;
    using pointer = typename std::vector<T, Allocator>::pointer;
    using const_pointer = typename std::vector<T, Allocator>::const_pointer;
    using iterator = typename std::vector<T, Allocator>::iterator;
    using const_iterator = typename std::vector<T, Allocator>::const_iterator;
    using reverse_iterator = typename std::vector<T, Allocator>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T, Allocator>::const_reverse_iterator;

    // 构造函数
    ThreadSafeVector() noexcept(std::is_nothrow_default_constructible_v<Allocator>) = default;
    
    explicit ThreadSafeVector(const Allocator& alloc) noexcept 
        : data_(alloc) {}
    
    explicit ThreadSafeVector(size_type count, const T& value = T(), const Allocator& alloc = Allocator())
        : data_(alloc) {
        try {
            data_.assign(count, value);
        } catch (...) {
            throw;
        }
    }
    
    template<typename InputIt>
    ThreadSafeVector(InputIt first, InputIt last, const Allocator& alloc = Allocator())
        : data_(alloc) {
        try {
            data_.assign(first, last);
        } catch (...) {
            throw;
        }
    }
    
    // 拷贝构造函数 - 改为锁定源对象后直接拷贝，避免中间状态
    ThreadSafeVector(const ThreadSafeVector& other)
        : data_(other.get_allocator()) {
        std::shared_lock<std::shared_mutex> read_lock(other.mutex_);
        data_ = other.data_;
    }
    
    // 移动构造函数 - 加锁源对象，保证原子移动
    ThreadSafeVector(ThreadSafeVector&& other) noexcept
        : data_(other.get_allocator()) {
        std::unique_lock<std::shared_mutex> other_lock(other.mutex_);
        data_ = std::move(other.data_);
    }
    
    ThreadSafeVector(std::initializer_list<T> init, const Allocator& alloc = Allocator())
        : data_(alloc) {
        try {
            data_.assign(init);
        } catch (...) {
            throw;
        }
    }

    // 析构函数
    ~ThreadSafeVector() = default;

    // 赋值操作 - 复制交换，强异常安全
    ThreadSafeVector& operator=(const ThreadSafeVector& other) {
        if (this != &other) {
            ThreadSafeVector temp(other);
            swap(temp);
        }
        return *this;
    }

    ThreadSafeVector& operator=(ThreadSafeVector&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> write_lock(mutex_);
            std::unique_lock<std::shared_mutex> other_write_lock(other.mutex_);
            data_ = std::move(other.data_);
        }
        return *this;
    }

    ThreadSafeVector& operator=(std::initializer_list<T> ilist) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        try {
            data_.assign(ilist);
        } catch (...) {
            throw;
        }
        return *this;
    }

    // 容量相关操作
    bool empty() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.empty();
    }

    size_type size() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.size();
    }

    size_type max_size() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.max_size();
    }

    size_type capacity() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.capacity();
    }

    void reserve(size_type new_cap) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.reserve(new_cap);
    }

    void shrink_to_fit() {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.shrink_to_fit();
    }

    // 元素访问操作 - 多种访问方式
    T get_copy(size_type pos) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.at(pos);
    }

    T at(size_type pos) const {
        return get_copy(pos);
    }

    T operator[](size_type pos) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_[pos];
    }

    T front() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.front();
    }

    T back() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.back();
    }

    // 带回调的安全访问（const，只读）
    template<typename F>
    auto access(size_type pos, F&& func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        return func(data_[pos]);
    }

    // 带回调的修改访问（非 const，可写）
    template<typename F>
    auto access(size_type pos, F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        return func(data_[pos]);
    }

    // 专用修改访问（保留）
    template<typename F>
    void modify(size_type pos, F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos >= data_.size()) {
            throw std::out_of_range("Index out of range");
        }
        func(data_[pos]);
    }

    // 获取分配器
    allocator_type get_allocator() const noexcept {
        return data_.get_allocator();
    }

    // 修改器操作
    void clear() noexcept {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.clear();
    }

    bool insert(size_type pos, const T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.insert(data_.begin() + pos, value);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool insert(size_type pos, T&& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.insert(data_.begin() + pos, std::move(value));
            return true;
        } catch (...) {
            return false;
        }
    }

    bool insert(size_type pos, size_type count, const T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.insert(data_.begin() + pos, count, value);
            return true;
        } catch (...) {
            return false;
        }
    }

    template<typename InputIt>
    bool insert(size_type pos, InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.insert(data_.begin() + pos, first, last);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool insert(size_type pos, std::initializer_list<T> ilist) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.insert(data_.begin() + pos, ilist);
            return true;
        } catch (...) {
            return false;
        }
    }

    template<typename... Args>
    bool emplace(size_type pos, Args&&... args) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos > data_.size()) {
            return false;
        }
        try {
            data_.emplace(data_.begin() + pos, std::forward<Args>(args)...);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool erase(size_type pos) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (pos >= data_.size()) {
            return false;
        }
        try {
            data_.erase(data_.begin() + pos);
            return true;
        } catch (...) {
            return false;
        }
    }

    bool erase(size_type first, size_type last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (first >= data_.size() || last > data_.size() || first >= last) {
            return false;
        }
        try {
            data_.erase(data_.begin() + first, data_.begin() + last);
            return true;
        } catch (...) {
            return false;
        }
    }

    void push_back(const T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.push_back(value);
    }

    void push_back(T&& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.push_back(std::move(value));
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.emplace_back(std::forward<Args>(args)...);
    }

    // 批量操作：减少锁竞争
    template<typename InputIt>
    void push_back_range(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.insert(data_.end(), first, last);
    }

    void pop_back_range(size_type count) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (count >= data_.size()) {
            data_.clear();
            return;
        }
        data_.resize(data_.size() - count);
    }

    // 改进：pop_back 返回被移除的元素
    std::optional<T> pop_back() {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        if (data_.empty()) {
            return std::nullopt;
        }
        T value = std::move(data_.back());
        data_.pop_back();
        return value;
    }

    // 使用移动语义返回数据，避免拷贝
    std::vector<T> move_vector() {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        return std::move(data_);
    }

    void resize(size_type count) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.resize(count);
    }

    void resize(size_type count, const value_type& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.resize(count, value);
    }

    void swap(ThreadSafeVector& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> write_lock(mutex_);
            std::unique_lock<std::shared_mutex> other_write_lock(other.mutex_);
            data_.swap(other.data_);
        }
    }

    // 迭代器操作 - 安全的回调式访问
    template<typename F>
    void with_iterators(F&& func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        func(data_.begin(), data_.end());
    }

    template<typename F>
    void with_iterators(F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        func(data_.begin(), data_.end());
    }

    // 转换为普通向量
    std::vector<T> to_vector() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_;
    }

    // 高级操作
    template<typename Predicate>
    void remove_if(Predicate pred) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.erase(std::remove_if(data_.begin(), data_.end(), pred), data_.end());
    }

    template<typename Predicate>
    std::optional<T> find_if(Predicate pred) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = std::find_if(data_.begin(), data_.end(), pred);
        if (it != data_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    template<typename Predicate>
    std::vector<T> filter(Predicate pred) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::vector<T> result;
        std::copy_if(data_.begin(), data_.end(), std::back_inserter(result), pred);
        return result;
    }

    template<typename Function>
    void for_each(Function func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::for_each(data_.begin(), data_.end(), func);
    }

    template<typename Function>
    auto transform(Function func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        using ResultType = std::invoke_result_t<Function, const T&>;
        std::vector<ResultType> result;
        result.reserve(data_.size());
        std::transform(data_.begin(), data_.end(), std::back_inserter(result), func);
        return result;
    }

    // 批量操作
    template<typename InputIt>
    void assign(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.assign(first, last);
    }

    void assign(size_type count, const T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.assign(count, value);
    }

    void assign(std::initializer_list<T> ilist) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.assign(ilist);
    }

    // 线程安全的快照操作
    std::vector<T> snapshot() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_;
    }

    // 原子操作
    bool try_push_back(const T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_, std::try_to_lock);
        if (write_lock.owns_lock()) {
            data_.push_back(value);
            return true;
        }
        return false;
    }

    bool try_pop_back(T& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_, std::try_to_lock);
        if (write_lock.owns_lock() && !data_.empty()) {
            value = std::move(data_.back());
            data_.pop_back();
            return true;
        }
        return false;
    }

    // 比较操作 - 逐元素比较，避免创建快照
    bool operator==(const ThreadSafeVector& other) const {
        if (this == &other) return true;
        
        std::shared_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
        std::lock(lock1, lock2);
        
        if (data_.size() != other.data_.size()) return false;
        
        for (size_type i = 0; i < data_.size(); ++i) {
            if (data_[i] != other.data_[i]) return false;
        }
        return true;
    }

    bool operator!=(const ThreadSafeVector& other) const {
        return !(*this == other);
    }

    bool operator<(const ThreadSafeVector& other) const {
        if (this == &other) return false;
        
        std::shared_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
        std::lock(lock1, lock2);
        
        return std::lexicographical_compare(
            data_.begin(), data_.end(),
            other.data_.begin(), other.data_.end()
        );
    }

    bool operator<=(const ThreadSafeVector& other) const {
        return !(other < *this);
    }

    bool operator>(const ThreadSafeVector& other) const {
        return other < *this;
    }

    bool operator>=(const ThreadSafeVector& other) const {
        return !(*this < other);
    }

    // 自定义比较：支持谓词
    template<typename Compare = std::equal_to<T>>
    bool equals(const ThreadSafeVector& other, Compare comp = {}) const {
        if (this == &other) return true;
        
        std::shared_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
        std::lock(lock1, lock2);
        
        if (data_.size() != other.data_.size()) return false;
        for (size_type i = 0; i < data_.size(); ++i) {
            if (!comp(data_[i], other.data_[i])) return false;
        }
        return true;
    }

private:
    mutable std::shared_mutex mutex_;
    std::vector<T, Allocator> data_;
};

// 非成员函数
template<typename T, typename Allocator>
void swap(ThreadSafeVector<T, Allocator>& lhs, ThreadSafeVector<T, Allocator>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace roc::base::containers
