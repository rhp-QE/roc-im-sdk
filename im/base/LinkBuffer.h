#ifndef NET_BUFFER_HPP
#define NET_BUFFER_HPP

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <cstring>
#include <optional>
#include <vector>


namespace roc::base {

/// 非线程安全
/// 由于提供了 异步写入接口。 需要使用者保证写入操作完成后再 执行下一次写入
class LinkBuffer : public std::enable_shared_from_this<LinkBuffer> {
public:
    // 内存块结构
    struct Block {
        size_t capacity;          // 块总容量
        size_t write_pos = 0;     // 写入位置
        size_t read_pos = 0;      // 消费位置
        std::unique_ptr<char[]> data; // 数据存储

        explicit Block(size_t size);
        void reset() noexcept;    // 重置块状态
        size_t writable() const noexcept; // 剩余可写空间
        size_t readable() const noexcept; // 剩余可读数据
        size_t refCount = 1; // 引用次数

        ~Block() {
            std::cout<<"[release block]"<<std::endl;
        }
    };

    struct ReadResult {

        friend class LinkBuffer;

        using BlockItorType = std::list<std::unique_ptr<Block>>::iterator;

        public:

            std::vector<boost::asio::const_buffer> buffers; // 数据缓冲区
            void release() const;
            std::string readString() const;
            uint32_t peekUnint32();
            size_t readBytes(char *buf, size_t size);
        
        private:
            
            void set_ref_block_iterators(const std::vector<BlockItorType>& vec) {
                ref_block_iterators_ = vec;
            }

            std::vector<BlockItorType> ref_block_iterators_; // 
            std::shared_ptr<LinkBuffer> buffer_;
    };

    explicit LinkBuffer(size_t init_block_size = 4096);
    
    // 核心接口
    void write(const void* data, size_t len);           // 用户主动写入数据（追加到末尾）

    // 同步写入
    void sync_write(size_t size, std::function<size_t(std::vector<boost::asio::mutable_buffer>& buffer)> sync_write_block);

    // 异步写入
    std::vector<boost::asio::mutable_buffer> prepare_buffers(size_t hint = 0); // 获取可写缓冲区
    void commit(size_t written);                        // 提交写入数据

    std::optional<ReadResult> get_read_buffers(size_t size = 0);           // 返回数据及区间游标

    size_t readable() const noexcept;                        // 当前可读数据总长度
    size_t writeable() const noexcept;

private:
    mutable std::mutex mutex_;
    std::list<std::unique_ptr<Block>> blocks_;
    size_t default_block_size_;
    std::list<std::unique_ptr<Block>>::iterator write_block_cursor_; // 空闲块遍历游标
    std::list<std::unique_ptr<Block>>::iterator read_block_cursor_; // 空闲块遍历游标

    void append_new_block(size_t hint);                 // 创建新块
    void append_new_block(std::unique_ptr<Block> block);
};

} /// namespace roc::base


template<typename T>
int find_index(const std::list<T>& lst, typename std::list<T>::iterator target) {
    int index = 0;
    for (auto it = lst.begin(); it != lst.end(); ++it, ++index) {
        if (it == target) {
            return index;
        }
    }
    return -1; // 未找到（目标迭代器不属于该 list）
}

#endif