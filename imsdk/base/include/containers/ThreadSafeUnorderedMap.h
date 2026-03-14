//
// ThreadSafeUnorderedMap.h
//
// 线程安全的 std::unordered_map 封装
// 提供完整的线程安全操作接口，支持读多写少的场景
//
// author: AI Assistant
// date: 2025-01-xx
// version: 1.0 - 基于ThreadSafeVector的经验
//

#pragma once

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <algorithm>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace roc::base::containers {

template<typename Key, typename T, typename Hash = std::hash<Key>, 
         typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<std::pair<const Key, T>>>
class ThreadSafeUnorderedMap {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const Key, T>;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using size_type = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::size_type;
    using difference_type = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::difference_type;
    using reference = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::reference;
    using const_reference = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_reference;
    using pointer = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::pointer;
    using const_pointer = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_pointer;
    using iterator = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::iterator;
    using const_iterator = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_iterator;
    using local_iterator = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::local_iterator;
    using const_local_iterator = typename std::unordered_map<Key, T, Hash, KeyEqual, Allocator>::const_local_iterator;

    // 构造函数 - 完整支持自定义分配器
    ThreadSafeUnorderedMap() noexcept(std::is_nothrow_default_constructible_v<Allocator>) = default;
    
    explicit ThreadSafeUnorderedMap(size_type bucket_count, const Hash& hash = Hash(), 
                                   const KeyEqual& equal = KeyEqual(), const Allocator& alloc = Allocator())
        : data_(bucket_count, hash, equal, alloc) {}
    
    explicit ThreadSafeUnorderedMap(const Allocator& alloc) noexcept 
        : data_(alloc) {}
    
    template<typename InputIt>
    ThreadSafeUnorderedMap(InputIt first, InputIt last, size_type bucket_count = 0,
                           const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), 
                           const Allocator& alloc = Allocator())
        : data_(first, last, bucket_count, hash, equal, alloc) {}
    
    // 复制构造函数 - 正确传播分配器
    ThreadSafeUnorderedMap(const ThreadSafeUnorderedMap& other)
        : data_(other.get_allocator()) {
        std::shared_lock<std::shared_mutex> read_lock(other.mutex_);
        data_ = other.data_;
    }
    
    // 复制构造函数 - 使用自定义分配器
    ThreadSafeUnorderedMap(const ThreadSafeUnorderedMap& other, const Allocator& alloc)
        : data_(alloc) {
        std::shared_lock<std::shared_mutex> read_lock(other.mutex_);
        data_ = other.data_;
    }
    
    // 移动构造函数 - 正确传播分配器
    ThreadSafeUnorderedMap(ThreadSafeUnorderedMap&& other) noexcept
        : data_(other.get_allocator()) {  // 只初始化分配器，不移动数据
        std::unique_lock<std::shared_mutex> other_lock(other.mutex_);
        data_ = std::move(other.data_);  // 在锁保护下移动数据
    }
    
    // 移动构造函数 - 使用自定义分配器
    ThreadSafeUnorderedMap(ThreadSafeUnorderedMap&& other, const Allocator& alloc) noexcept
        : data_(alloc) {  // 使用指定的分配器初始化
        std::unique_lock<std::shared_mutex> other_lock(other.mutex_);
        data_ = std::move(other.data_);  // 在锁保护下移动数据
    }
    
    ThreadSafeUnorderedMap(std::initializer_list<value_type> init, size_type bucket_count = 0,
                           const Hash& hash = Hash(), const KeyEqual& equal = KeyEqual(), 
                           const Allocator& alloc = Allocator())
        : data_(init, bucket_count, hash, equal, alloc) {}

    // 析构函数
    ~ThreadSafeUnorderedMap() = default;

    // 赋值操作 - 复制交换，强异常安全
    ThreadSafeUnorderedMap& operator=(const ThreadSafeUnorderedMap& other) {
        if (this != &other) {
            ThreadSafeUnorderedMap temp(other);
            swap(temp);
        }
        return *this;
    }

    ThreadSafeUnorderedMap& operator=(ThreadSafeUnorderedMap&& other) noexcept {
        if (this != &other) {
            // 使用 copy-and-swap 模式，确保分配器正确传播
            ThreadSafeUnorderedMap temp(std::move(other));
            swap(temp);
        }
        return *this;
    }

    ThreadSafeUnorderedMap& operator=(std::initializer_list<value_type> ilist) {
        if (assign(ilist)) {
            return *this;
        } else {
            throw std::runtime_error("Failed to assign initializer list");
        }
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

    size_type bucket_count() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.bucket_count();
    }

    size_type max_bucket_count() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.max_bucket_count();
    }

    size_type bucket_size(size_type n) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.bucket_size(n);
    }

    size_type bucket(const Key& key) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.bucket(key);
    }

    float load_factor() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.load_factor();
    }

    float max_load_factor() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.max_load_factor();
    }

    void max_load_factor(float z) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.max_load_factor(z);
    }

    void rehash(size_type count) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.rehash(count);
    }

    void reserve(size_type count) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.reserve(count);
    }

    // 元素访问操作 - 多种安全方式
    // 1. 安全的副本访问
    std::optional<T> at(const Key& key) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 2. 带默认值的访问, 如果没有找到会把传入的默认值移动到容器内
    std::pair<bool, T> at(const Key& key, T&& default_value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = data_.find(key);
        if (it == data_.end()) {
            it = data_.emplace(key, std::move(default_value)).first;
            return {false, it->second};
        }
        return {true, it->second};
    }

    // 3. 带回调的安全访问（const，只读）
    template<typename F>
    auto access(const Key& key, F&& func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::out_of_range("Key not found");
        }
        return func(it->second);
    }

    // 4. 带回调的修改访问（非 const，可写）
    template<typename F>
    auto access(const Key& key, F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::out_of_range("Key not found");
        }
        return func(it->second);
    }

    // 5. 专用修改访问 - 键必须存在
    template<typename F>
    bool modify(const Key& key, F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = data_.find(key);
        if (it == data_.end()) {
            return false;
        }
        func(it->second);
        return true;
    }

    // 6. 智能修改访问 - 键不存在时自动创建
    template<typename F>
    T modify_or_create(const Key& key, F&& func, const T& default_value = T{}) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = data_.find(key);
        if (it == data_.end()) {
            // 键不存在，创建新键值对
            auto result = data_.emplace(key, default_value);
            it = result.first;
        }
        // 调用用户函数修改值
        func(it->second);
        return it->second;
    }

    // 8. 获取分配器
    allocator_type get_allocator() const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.get_allocator();
    }

    // 9. 获取哈希函数和比较函数
    hasher hash_function() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.hash_function();
    }

    key_equal key_eq() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.key_eq();
    }

    // 10. 分配器相关操作
    /// @brief 检查两个容器的分配器是否相等
    bool allocator_equal(const ThreadSafeUnorderedMap& other) const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::shared_lock<std::shared_mutex> other_read_lock(other.mutex_);
        return data_.get_allocator() == other.data_.get_allocator();
    }

    /// @brief 使用新分配器创建副本
    ThreadSafeUnorderedMap with_allocator(const Allocator& alloc) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return ThreadSafeUnorderedMap(*this, alloc);
    }

    /// @brief 使用新分配器移动数据
    ThreadSafeUnorderedMap with_allocator(const Allocator& alloc) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        return ThreadSafeUnorderedMap(std::move(*this), alloc);
    }

    // 修改器操作
    void clear() noexcept {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.clear();
    }

    // 插入操作 - 返回插入结果，不返回迭代器（避免悬空指针）
    std::pair<bool, T> insert(const value_type& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.insert(value);
        return {result.second, result.first->second};
    }

    std::pair<bool, T> insert(value_type&& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.insert(std::move(value));
        return {result.second, result.first->second};
    }

    template<typename P>
    std::pair<bool, T> insert(P&& value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.insert(std::forward<P>(value));
        return {result.second, result.first->second};
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.insert(first, last);
    }

    void insert(std::initializer_list<value_type> ilist) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        data_.insert(ilist);
    }

    // 插入或赋值操作 - 返回操作结果，不返回迭代器
    template<typename M>
    std::pair<bool, T> insert_or_assign(const Key& key, M&& obj) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.insert_or_assign(key, std::forward<M>(obj));
        return {result.second, result.first->second};
    }

    template<typename M>
    std::pair<bool, T> insert_or_assign(Key&& key, M&& obj) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.insert_or_assign(std::move(key), std::forward<M>(obj));
        return {result.second, result.first->second};
    }

    // 尝试插入操作 - 返回操作结果，不返回迭代器
    template<typename... Args>
    std::pair<bool, T> try_emplace(const Key& key, Args&&... args) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.try_emplace(key, std::forward<Args>(args)...);
        return {result.second, result.first->second};
    }

    template<typename... Args>
    std::pair<bool, T> try_emplace(Key&& key, Args&&... args) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto result = data_.try_emplace(std::move(key), std::forward<Args>(args)...);
        return {result.second, result.first->second};
    }

    // 删除操作 - 不返回迭代器（避免悬空指针）
    size_type erase(const Key& key) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        return data_.erase(key);
    }

    bool erase(const Key& key, T& removed_value) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            removed_value = it->second;
            data_.erase(it);
            return true;
        }
        return false;
    }

    // 范围删除 - 返回删除的元素数量
    size_type erase_range(const std::vector<Key>& keys) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        size_type count = 0;
        for (const auto& key : keys) {
            count += data_.erase(key);
        }
        return count;
    }

    // 查找操作
    bool contains(const Key& key) const noexcept {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.find(key) != data_.end();
    }

    size_type count(const Key& key) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_.count(key);
    }

    std::optional<T> find(const Key& key) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // 批量操作：减少锁竞争，返回插入结果
    template<typename InputIt>
    std::pair<size_type, size_type> insert_range(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        size_type before_size = data_.size();
        data_.insert(first, last);
        size_type after_size = data_.size();
        return {before_size, after_size - before_size};
    }

    template<typename Container>
    std::pair<size_type, size_type> insert_container(const Container& container) {
        return insert_range(container.begin(), container.end());
    }

    // 安全的迭代器访问 - 只读版本
    /// @brief 在持有读锁的情况下执行回调函数
    /// @param func 回调函数，接收 const 开始和结束迭代器
    /// @warning 在回调函数中不要执行可能引起重分配的操作，避免死锁
    /// @warning 回调函数中不要调用可能获取写锁的方法
    /// @note 此方法提供只读访问，多个线程可以同时调用
    template<typename F>
    void with_iterators(F&& func) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        func(data_.begin(), data_.end());
    }

    // 安全的迭代器访问 - 读写版本
    /// @brief 在持有写锁的情况下执行回调函数
    /// @param func 回调函数，接收开始和结束迭代器
    /// @warning 在回调函数中不要执行可能引起重分配的操作，避免死锁
    /// @warning 回调函数中不要调用可能获取读锁或写锁的方法
    /// @warning 回调函数中不要调用可能修改容器大小的操作（如 insert, erase, clear 等）
    /// @note 此方法提供读写访问，会阻塞其他所有线程的访问
    template<typename F>
    void with_iterators(F&& func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        func(data_.begin(), data_.end());
    }

    // 转换为普通容器
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> to_map() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_;
    }

    // 使用移动语义返回数据，避免拷贝
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> move_map() {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        return std::move(data_);
    }

    // 线程安全的快照操作
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> snapshot() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return data_;
    }

    // 高级操作
    template<typename Predicate>
    void remove_if(Predicate pred) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        for (auto it = data_.begin(); it != data_.end();) {
            if (pred(*it)) {
                it = data_.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<typename Predicate>
    std::optional<value_type> find_if(Predicate pred) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        auto it = std::find_if(data_.begin(), data_.end(), pred);
        if (it != data_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    template<typename Predicate>
    std::vector<value_type> filter(Predicate pred) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::vector<value_type> result;
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
        using ResultType = std::invoke_result_t<Function, const value_type&>;
        std::vector<ResultType> result;
        result.reserve(data_.size());
        std::transform(data_.begin(), data_.end(), std::back_inserter(result), func);
        return result;
    }

    // 批量操作 - 强异常安全
    template<typename InputIt>
    bool assign(InputIt first, InputIt last) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        try {
            std::unordered_map<Key, T, Hash, KeyEqual, Allocator> temp(data_.get_allocator());
            temp.insert(first, last);
            data_.swap(temp);  // 使用 swap 保证强异常安全
            return true;
        } catch (...) {
            return false;
        }
    }

    bool assign(std::initializer_list<value_type> ilist) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        try {
            std::unordered_map<Key, T, Hash, KeyEqual, Allocator> temp(data_.get_allocator());
            temp.insert(ilist);
            data_.swap(temp);  // 使用 swap 保证强异常安全
            return true;
        } catch (...) {
            return false;
        }
    }

    void swap(ThreadSafeUnorderedMap& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> write_lock(mutex_);
            std::unique_lock<std::shared_mutex> other_write_lock(other.mutex_);
            data_.swap(other.data_);
        }
    }

    // 比较操作 - 逐元素比较，避免创建快照
    bool operator==(const ThreadSafeUnorderedMap& other) const {
        if (this == &other) return true;
        
        std::shared_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
        std::lock(lock1, lock2);
        
        if (data_.size() != other.data_.size()) return false;
        
        // 由于大小相等，只需要检查 this 中的每个元素是否在 other 中存在且值相等
        // 如果 this 中的所有元素都在 other 中且值相等，那么 other 中不可能有额外的元素
        for (const auto& pair : data_) {
            auto it = other.data_.find(pair.first);
            if (it == other.data_.end() || it->second != pair.second) {
                return false;
            }
        }
        
        return true;
    }

    bool operator!=(const ThreadSafeUnorderedMap& other) const {
        return !(*this == other);
    }

    // 自定义比较：支持谓词
    template<typename Compare = std::equal_to<value_type>>
    bool equals(const ThreadSafeUnorderedMap& other, Compare comp = {}) const {
        if (this == &other) return true;
        
        std::shared_lock<std::shared_mutex> lock1(mutex_, std::defer_lock);
        std::shared_lock<std::shared_mutex> lock2(other.mutex_, std::defer_lock);
        std::lock(lock1, lock2);
        
        if (data_.size() != other.data_.size()) return false;
        
        // 由于大小相等，只需要检查 this 中的每个元素是否在 other 中存在且满足比较条件
        // 如果 this 中的所有元素都在 other 中且满足比较条件，那么 other 中不可能有额外的元素
        for (const auto& pair : data_) {
            auto it = other.data_.find(pair.first);
            if (it == other.data_.end() || !comp(pair, *it)) {
                return false;
            }
        }
        
        return true;
    }

    // 统计信息
    std::pair<size_type, float> get_stats() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        return {data_.size(), data_.load_factor()};
    }

    // 性能优化：批量查找
    template<typename Container>
    std::vector<std::pair<Key, T>> find_multiple(const Container& keys) const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::vector<std::pair<Key, T>> result;
        result.reserve(keys.size());
        
        for (const auto& key : keys) {
            auto it = data_.find(key);
            if (it != data_.end()) {
                result.emplace_back(it->first, it->second);
            }
        }
        return result;
    }

    // 性能优化：条件更新
    template<typename Predicate, typename UpdateFunc>
    size_type update_if(Predicate pred, UpdateFunc update_func) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        size_type count = 0;
        
        for (auto& pair : data_) {
            if (pred(pair)) {
                update_func(pair.second);
                ++count;
            }
        }
        return count;
    }

    // 获取所有键
    std::vector<Key> keys() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::vector<Key> result;
        result.reserve(data_.size());
        for (const auto& pair : data_) {
            result.push_back(pair.first);
        }
        return result;
    }

    // 获取所有值
    std::vector<T> values() const {
        std::shared_lock<std::shared_mutex> read_lock(mutex_);
        std::vector<T> result;
        result.reserve(data_.size());
        for (const auto& pair : data_) {
            result.push_back(pair.second);
        }
        return result;
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<Key, T, Hash, KeyEqual, Allocator> data_;
};

// 非成员函数
template<typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
void swap(ThreadSafeUnorderedMap<Key, T, Hash, KeyEqual, Allocator>& lhs, 
          ThreadSafeUnorderedMap<Key, T, Hash, KeyEqual, Allocator>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace roc::base::containers
