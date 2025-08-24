#pragma once
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;

void beast_buf_test() {
    // 创建第一个 flat_buffer
    beast::flat_buffer buffer1;
    
    // 向缓冲区填充一些数据
    std::string test_data = "Hello, Boost.Beast!";
    asio::buffer_copy(buffer1.prepare(test_data.size()), 
                     asio::buffer(test_data));
    buffer1.commit(test_data.size());
    
    // 输出原始缓冲区的信息和内容
    std::cout << "Buffer1 size: " << buffer1.size() << std::endl;
    std::cout << "Buffer1 capacity: " << buffer1.capacity() << std::endl;
    std::cout << "Buffer1 content: " << beast::make_printable(buffer1.data()) << std::endl;
    std::cout << "Buffer1 data address: " << static_cast<const void*>(buffer1.data().data()) << std::endl;
    std::cout << "---\n";
    
    // 执行拷贝操作 - 这将创建一个深拷贝
    beast::flat_buffer buffer2 = buffer1;
    
    // 输出拷贝后缓冲区的信息和内容
    std::cout << "After copy:\n";
    std::cout << "Buffer2 size: " << buffer2.size() << std::endl;
    std::cout << "Buffer2 capacity: " << buffer2.capacity() << std::endl;
    std::cout << "Buffer2 content: " << beast::make_printable(buffer2.data()) << std::endl;
    std::cout << "Buffer2 data address: " << static_cast<const void*>(buffer2.data().data()) << std::endl;
    std::cout << "---\n";
    
    // 证明两个缓冲区有相同内容但不同内存地址
    std::cout << "Comparison:\n";
    std::cout << "Contents are equal: " 
              << (beast::buffers_to_string(buffer1.data()) == beast::buffers_to_string(buffer2.data())) 
              << std::endl;
    std::cout << "Memory addresses are different: " 
              << (buffer1.data().data() != buffer2.data().data()) 
              << std::endl;
    std::cout << "---\n";
    
    // 修改原始缓冲区
    std::string modified_data = "Modified data!";
    buffer1.consume(buffer1.size()); // 清空缓冲区
    asio::buffer_copy(buffer1.prepare(modified_data.size()), 
                     asio::buffer(modified_data));
    buffer1.commit(modified_data.size());
    
    // 证明修改原始缓冲区不影响拷贝的缓冲区
    std::cout << "After modifying buffer1:\n";
    std::cout << "Buffer1 content: " << beast::make_printable(buffer1.data()) << std::endl;
    std::cout << "Buffer2 content: " << beast::make_printable(buffer2.data()) << std::endl;
    std::cout << "Contents are different: " 
              << (beast::buffers_to_string(buffer1.data()) != beast::buffers_to_string(buffer2.data())) 
              << std::endl;
    
    return ;
}