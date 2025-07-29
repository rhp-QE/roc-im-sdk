#include "LinkBuffer.h"
#include <algorithm>
#include <boost/asio/buffer.hpp>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <im/base/Utility.h>

using namespace::roc::base;

int id = 0;

LinkBuffer::Block::Block(size_t size) 
    : capacity(size), data(new char[size]) {}

void LinkBuffer::Block::reset() noexcept {
    write_pos = 0;
    read_pos = 0;
}

size_t LinkBuffer::Block::writable() const noexcept {
    return capacity - write_pos;
}

size_t LinkBuffer::Block::readable() const noexcept {
    return write_pos - read_pos;
}

void LinkBuffer::ReadResult::release() const {
    for (auto& ref_block_iterator : ref_block_iterators_) {
        if (ref_block_iterator == buffer_->blocks_.end()) {
            continue;
        }
        
        --(*ref_block_iterator)->refCount;

        std::cout<<"[block refcount]"<<(*ref_block_iterator)->refCount<<std::endl;

        
        std::cout<<"*[reuse block]"<<buffer_<<" "<<buffer_->readable()<<" "<<buffer_->writeable()<<" "<<buffer_->blocks_.size()<<" "<<find_index(buffer_->blocks_, buffer_->read_block_cursor_)<<" "<<find_index(buffer_->blocks_, buffer_->write_block_cursor_)<<std::endl;
        if ((*ref_block_iterator)->refCount == 0) {
            std::cout<<"-[reuse block]"<<buffer_<<" "<<buffer_->readable()<<" "<<buffer_->writeable()<<" "<<buffer_->blocks_.size()<<" "<<find_index(buffer_->blocks_, buffer_->read_block_cursor_)<<" "<<find_index(buffer_->blocks_, buffer_->write_block_cursor_)<<std::endl;
            std::cout<<"[reuse block addr]"<<(*ref_block_iterator).get()<<std::endl;
            int i = 0;
            for(auto it = buffer_->blocks_.begin() ; it!=buffer_->blocks_.end() && i<30;++i, ++it) {
                std::cout<<(*it)->refCount<<" ";
            }
            std::cout<<std::endl;
            auto block = std::move(*ref_block_iterator);
            buffer_->blocks_.erase(ref_block_iterator);
            buffer_->append_new_block(std::move(block));
            std::cout<<"+[reuse block]"<<buffer_<<" "<<buffer_->readable()<<" "<<buffer_->writeable()<<" "<<buffer_->blocks_.size()<<" "<<find_index(buffer_->blocks_, buffer_->read_block_cursor_)<<" "<<find_index(buffer_->blocks_, buffer_->write_block_cursor_)<<std::endl;
            std::cout<<"[back block addr]"<<buffer_->blocks_.back().get()<<std::endl;
        }
    }
}

std::string LinkBuffer::ReadResult::readString() const {
    std::string result;

    // 计算所有缓冲区的总字节数
    size_t total_size = 0;

    for (const auto& buffer : buffers) {
        total_size += boost::asio::buffer_size(buffer);
    }

    // 预分配内存
    result.reserve(total_size);

    // 拼接数据
    for (const auto& buffer : buffers) {
        const char* data = static_cast<const char *>(buffer.data());
        size_t length = boost::asio::buffer_size(buffer);
        result.append(data, length);
    }
    
    return result;
}

size_t LinkBuffer::ReadResult::readBytes(char* buf, size_t size) {
    // 计算所有缓冲区的总字节数
    size_t total_size = 0;

    for (const auto& buffer : buffers) {
        total_size += boost::asio::buffer_size(buffer);
    }

    size_t pos = 0;
    for (const auto& buffer : buffers) {
        const char* data = static_cast<const char *>(buffer.data());
        size_t length = std::min(boost::asio::buffer_size(buffer), size);

        std::copy(data, data + length, buf + pos);
        pos += length;

        size -= length;
        if (size == 0) {
            break;
        }
    }
    return pos;
}

uint32_t LinkBuffer::ReadResult::peekUnint32() {
    char number_bytes[4];
    readBytes(number_bytes, 4);

    return roc::base::util::decode_uint32_LE(number_bytes);
}

LinkBuffer::LinkBuffer(size_t init_block_size)
    : default_block_size_(init_block_size)
{
    blocks_.emplace_back(std::make_unique<Block>(init_block_size));
    write_block_cursor_ = blocks_.begin();
    read_block_cursor_ = blocks_.begin();
}

// 用户主动写入数据（通过 prepare_buffers 和 commit 实现追加）
void LinkBuffer::write(const void* data, size_t len) {
    std::cout<<"[call write]"<<std::endl;
    const char* src = static_cast<const char*>(data);
    auto bufs = prepare_buffers(len);

    size_t written = 0;
    for (const auto& buf : bufs) {
        if (len == written) {
            break;
        }
        std::cout<<"[buf size]"<<buf.size()<<std::endl;
        const size_t copy_size = std::min(buf.size(), len - written);
        std::memcpy(buf.data(), src + written, copy_size);
        written += copy_size;
    }

    commit(written);
}

/// 准备写入空间
/// size = 0 : 返回剩余的所有空间
std::vector<boost::asio::mutable_buffer> LinkBuffer::prepare_buffers(size_t size) {
    std::cout<<"[block count] "<<this<<" "<<blocks_.size()<<std::endl;
    /// 保证有足够空间
    if (writeable() < size || writeable() == 0) {
        append_new_block(std::max(default_block_size_, size));
    }

    size = (size == 0) ? writeable() : size;

    std::cout<<"[prepare]"<<size<<std::endl;
    std::vector<boost::asio::mutable_buffer> bufs;
    std::list<std::unique_ptr<Block>>::const_iterator it = write_block_cursor_;

    size_t prepare_size = 0;

    for(; it != blocks_.end(); ++it) {
        auto& block = *it;

        size_t chunk = std::min(size, block->writable());

        bufs.emplace_back(block->data.get() + block->write_pos, chunk);
        prepare_size += chunk;
        size -= chunk;
        if (size == 0) {
            break;
        }
    }

    ++id;
    std::cout<<"[call prepare buffer]"<<this<<" "<<prepare_size<<" "<<id<<std::endl;

    return bufs;
}

/// 写入后确认提交
void LinkBuffer::commit(size_t written) {
    if (written > writeable()) {
        std::cout<<"[error] commit size over left size"<<std::endl;
        return;
    }

    std::cout<<"[call commit] :"<<this<<" "<<written<<" "<<id<<std::endl;
    std::list<std::unique_ptr<Block>>::iterator it = write_block_cursor_;

    for(; it != blocks_.end(); ++it) {
        auto& block = *it;
        size_t chunk = std::min(block->writable(), written);
        block->write_pos += chunk;
        written -= chunk;

        std::cout<<"[block commit count]"<<chunk<<std::endl;

        if (written == 0) {
            break;
        }
    }

    // 确保 write_cursor 指向的空间始终可写
    if (it != blocks_.end() && (*it)->writable() == 0) {
        ++ it;
    }

    write_block_cursor_ = it;
}


void LinkBuffer::sync_write(size_t size, std::function<size_t(std::vector<boost::asio::mutable_buffer>& buffer)> sync_write_block) {
    auto buf = prepare_buffers(size);
    size_t written_size = roc::base::util::safe_invoke_block(sync_write_block, buf);
    commit(written_size);
}

/// 获取数据
/// size < readable : 返回 std::nullopt
std::optional<LinkBuffer::ReadResult> LinkBuffer::get_read_buffers(size_t size) {

    if (size > readable() || readable() == 0) {
        std::cerr << "[base::buffer::get_read_buffer] readable count is not enough" << std::endl;
        return std::nullopt;
    }

    std::cout<<"[left read count]"<<readable()<<" [block count]"<<blocks_.size()<<" "<<this<<std::endl;

    LinkBuffer::ReadResult result;
    size = (size == 0) ? readable() : size;

    // 从全局游标位置开始读取
    auto it = read_block_cursor_;

    // 记录本次读取的起始位置
    std::vector<boost::asio::const_buffer> buffers;
    std::vector<ReadResult::BlockItorType> ref_block_iterators;

    for (; it != blocks_.end(); ++it) {
        auto& block = *it;
        const size_t chunk = std::min(block->readable(), size);
        buffers.emplace_back(block->data.get() + block->read_pos, chunk);
        ref_block_iterators.emplace_back(it);
        block->read_pos += chunk;
        ++block->refCount;
        size -= chunk;

        std::cout<<"[read from block count]"<<chunk<<std::endl;

        /// 该节点引用计数 - 1
        if (block->readable() == 0 && block->writable() == 0) {
            --block->refCount;
            std::cout<<block->refCount<<" "<<this<<std::endl;
        }

        if (size == 0) {
            break;
        }
    }

    if (it != blocks_.end() && (*it)->readable() == 0 && (*it)->writable() == 0) {
        ++it;
    }
    read_block_cursor_ = it;

    result.set_ref_block_iterators(ref_block_iterators);
    result.buffers = buffers;
    result.buffer_ = shared_from_this();


    return result;
}

size_t LinkBuffer::readable() const noexcept {
    std::list<std::unique_ptr<Block>>::const_iterator it = read_block_cursor_;
    size_t total = 0;

    for (; it != blocks_.end(); ++it) {
        auto& block = *it;
        total += block->readable();
    }

    return total;
}

size_t LinkBuffer::writeable() const noexcept {
    std::list<std::unique_ptr<Block>>::const_iterator it = write_block_cursor_;
    size_t total = 0;

    for (; it != blocks_.end(); ++it) {
        auto& block = *it;
        total += block->writable();
    }

    return total;
}

void LinkBuffer::append_new_block(size_t hint) {
    std::cout<<"malloc new block] "<<this<<std::endl;

    if (blocks_.size() > 30) {
        std::cout<<"[error]"<<this<<" "<< writeable()<<" "<<readable()<<" "<<blocks_.size()<<std::endl;
        std::cout<<"[error]"<<this<<" "<<find_index(blocks_, write_block_cursor_)<<" "<<find_index(blocks_, read_block_cursor_)<<std::endl;
    }

    bool end_write = write_block_cursor_ == blocks_.end();
    bool end_read = read_block_cursor_ == blocks_.end();
    const size_t new_size = std::max(default_block_size_, hint);
    blocks_.emplace_back(std::make_unique<Block>(new_size));

    if (end_write) {
        write_block_cursor_ = --blocks_.end();
    }

    if (end_read) {
        read_block_cursor_ = --blocks_.end();
    }
}

void LinkBuffer::append_new_block(std::unique_ptr<Block> block) {
    block->reset();
    block->refCount = 1;
    bool end_write = write_block_cursor_ == blocks_.end();
    bool end_read = read_block_cursor_ == blocks_.end();
    blocks_.emplace_back(std::move(block));
    std::cout<<"reuse block count]"<<blocks_.back()->writable()<<std::endl;

    /// 读、写 位置游标在末尾 需要重置    
    if (end_write) {
        write_block_cursor_ = --blocks_.end();
        std::cout<<"[reset write]"<<std::endl;
    }

    if (end_read) {
        read_block_cursor_ = --blocks_.end();
        std::cout<<"[reset read]"<<std::endl;
    }
}