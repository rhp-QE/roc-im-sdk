#include <iostream>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "imsdk/base/include/network/HttpClient.h"

using namespace roc::network;
namespace net = boost::asio;

net::awaitable<void> test_http_client(net::io_context& io_context) {
    try {
        std::cout << "=== HTTP客户端测试 ===" << std::endl;
        
        // 创建HTTP客户端
        auto config = HttpClientConfig{}
            .timeout(std::chrono::seconds(10))
            .ssl_verification(false)  // 测试时暂时关闭SSL验证
            .user_agent_string("ROC-HttpClient-Test/1.0");
        
        auto client = create_http_client(io_context, config);
        
        // 测试1: 简单GET请求
        std::cout << "\n1. 测试简单GET请求..." << std::endl;
        auto result1 = co_await client->get("http://httpbin.org/get")
            .header("X-Test", "ROC-Test")
            .execute();
        
        if (result1 && result1->is_success()) {
            std::cout << "✅ GET请求成功" << std::endl;
            std::cout << "   状态码: " << result1->status_code() << std::endl;
            std::cout << "   响应大小: " << result1->content_length() << " bytes" << std::endl;
        } else if (result1) {
            std::cout << "❌ GET请求失败，HTTP状态: " << result1->status_code() << std::endl;
        } else {
            std::cout << "❌ GET请求网络错误: " << result1.error().message << std::endl;
        }
        
        // 测试2: POST JSON数据
        std::cout << "\n2. 测试POST JSON数据..." << std::endl;
        boost::json::object test_data;
        test_data["name"] = "ROC-Test";
        test_data["version"] = "1.0";
        test_data["timestamp"] = std::time(nullptr);
        
        auto result2 = co_await client->post("http://httpbin.org/post")
            .body(boost::json::value(test_data))
            .execute();
        
        if (result2 && result2->is_success()) {
            std::cout << "✅ POST请求成功" << std::endl;
            try {
                auto response_json = result2->to_json();
                std::cout << "   JSON响应解析成功" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "   JSON解析失败: " << e.what() << std::endl;
            }
        } else if (result2) {
            std::cout << "❌ POST请求失败，HTTP状态: " << result2->status_code() << std::endl;
        } else {
            std::cout << "❌ POST请求网络错误: " << result2.error().message << std::endl;
        }
        
        // 测试3: 带查询参数的GET请求
        std::cout << "\n3. 测试带查询参数的GET请求..." << std::endl;
        auto result3 = co_await client->get("http://httpbin.org/get")
            .query_param("test", "roc")
            .query_param("version", "1.0")
            .execute();
        
        if (result3 && result3->is_success()) {
            std::cout << "✅ 带参数GET请求成功" << std::endl;
        } else if (result3) {
            std::cout << "❌ 带参数GET请求失败，HTTP状态: " << result3->status_code() << std::endl;
        } else {
            std::cout << "❌ 带参数GET请求网络错误: " << result3.error().message << std::endl;
        }
        
        // 测试4: 便利方法
        std::cout << "\n4. 测试便利方法..." << std::endl;
        boost::json::object query_data;
        query_data["search"] = "test";
        query_data["limit"] = 5;
        
        auto result4 = co_await client->get_json("http://httpbin.org/get", 
                                                boost::json::value(query_data));
        
        if (result4 && result4->is_success()) {
            std::cout << "✅ 便利方法get_json成功" << std::endl;
        } else if (result4) {
            std::cout << "❌ 便利方法get_json失败，HTTP状态: " << result4->status_code() << std::endl;
        } else {
            std::cout << "❌ 便利方法get_json网络错误: " << result4.error().message << std::endl;
        }
        
        std::cout << "\n=== HTTP客户端测试完成 ===" << std::endl;
        
        // 清理资源
        client->shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "测试执行错误: " << e.what() << std::endl;
    }
}

void run_http_test() {
    try {
        net::io_context io_context;
        
        // 运行测试协程
        net::co_spawn(io_context, test_http_client(io_context), net::detached);
        
        // 运行事件循环
        io_context.run();
        
    } catch (const std::exception& e) {
        std::cerr << "HTTP测试错误: " << e.what() << std::endl;
    }
} 