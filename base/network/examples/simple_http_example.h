#include <iostream>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include "base/network/include/HttpClient.h"

using namespace roc::network;
namespace net = boost::asio;

net::awaitable<void> http_examples(net::io_context& io_context) {
    try {
        
        // 创建HTTP客户端
        auto config = HttpClientConfig{}
            .timeout(std::chrono::seconds(30))
            .ssl_verification(true)
            .user_agent_string("SimpleHttpClient/1.0");
        
        auto client = create_http_client(io_context, config);
        
        std::cout << "=== 简单HTTP客户端示例 ===" << std::endl;
        
        // 1. 基本GET请求
        std::cout << "\n1. 基本GET请求:" << std::endl;
        auto result1 = co_await client->get("https://httpbin.org/get")
            .header("X-Test", "example")
            .execute();
        
        if (result1 && result1->is_success()) {
            std::cout << "状态码: " << result1->status_code() << std::endl;
            std::cout << "响应大小: " << result1->content_length() << " bytes" << std::endl;
        } else if (result1) {
            std::cout << "请求失败，状态码: " << result1->status_code() << std::endl;
        } else {
            std::cout << "网络错误: " << result1.error().message << std::endl;
        }
        
        // 2. 带查询参数的GET请求
        std::cout << "\n2. 带查询参数的GET请求:" << std::endl;
        auto result2 = co_await client->get("https://httpbin.org/get")
            .query_param("name", "Alice")
            .query_param("age", "30")
            .query_param("active", "true")
            .execute();
        
        if (result2 && result2->is_success()) {
            std::cout << "请求成功，响应: " << result2->text().substr(0, 200) << "..." << std::endl;
        }
        
        // 3. POST JSON数据
        std::cout << "\n3. POST JSON数据:" << std::endl;
        boost::json::object post_data;
        post_data["username"] = "john_doe";
        post_data["email"] = "john@example.com";
        post_data["profile"] = boost::json::object{
            {"firstName", "John"},
            {"lastName", "Doe"}
        };
        
        auto result3 = co_await client->post("https://httpbin.org/post")
            .body(boost::json::value(post_data))
            .timeout(std::chrono::seconds(15))
            .execute();
        
        if (result3 && result3->is_success()) {
            std::cout << "POST请求成功" << std::endl;
            
            // 解析JSON响应
            try {
                auto json_response = result3->to_json();
                std::cout << "响应JSON解析成功" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "JSON解析失败: " << e.what() << std::endl;
            }
        }
        
        // 4. 使用便利方法
        std::cout << "\n4. 使用便利方法:" << std::endl;
        boost::json::object query_data;
        query_data["search"] = "test";
        query_data["limit"] = 10.0;
        
        auto result4 = co_await client->get_json("https://httpbin.org/get", 
                                                boost::json::value(query_data));
        
        if (result4 && result4->is_success()) {
            std::cout << "便利方法GET请求成功" << std::endl;
        }
        
        boost::json::object simple_data;
        simple_data["message"] = "Hello, World!";
        
        auto result5 = co_await client->post_json("https://httpbin.org/post", 
                                                 boost::json::value(simple_data));
        
        if (result5 && result5->is_success()) {
            std::cout << "便利方法POST请求成功" << std::endl;
        }
        
        // 5. 认证示例
        std::cout << "\n5. 认证示例:" << std::endl;
        auto result6 = co_await client->get("https://httpbin.org/bearer")
            .bearer_token("test-token-123")
            .execute();
        
        if (result6) {
            std::cout << "Bearer认证请求完成，状态: " << result6->status_code() << std::endl;
        }
        
        // 6. 不同HTTP方法
        std::cout << "\n6. 不同HTTP方法:" << std::endl;
        
        // PUT请求
        auto put_result = co_await client->put("https://httpbin.org/put")
            .body(boost::json::value(boost::json::object{{"data", "updated"}}))
            .execute();
        
        // PATCH请求
        auto patch_result = co_await client->patch("https://httpbin.org/patch")
            .body(boost::json::value(boost::json::object{{"field", "patched"}}))
            .execute();
        
        // DELETE请求
        auto delete_result = co_await client->delete_request("https://httpbin.org/delete")
            .header("X-Reason", "Testing")
            .execute();
        
        std::cout << "PUT状态: " << (put_result && put_result->is_success() ? "成功" : "失败") << std::endl;
        std::cout << "PATCH状态: " << (patch_result && patch_result->is_success() ? "成功" : "失败") << std::endl;
        std::cout << "DELETE状态: " << (delete_result && delete_result->is_success() ? "成功" : "失败") << std::endl;
        
        // 7. 回调方式
        std::cout << "\n7. 回调方式:" << std::endl;
        client->get("https://httpbin.org/delay/1")
            .timeout(std::chrono::seconds(5))
            .execute([](std::expected<HttpResponse, HttpError> result) {
                if (result && result->is_success()) {
                    std::cout << "回调请求成功，状态: " << result->status_code() << std::endl;
                } else if (result) {
                    std::cout << "回调请求失败，状态: " << result->status_code() << std::endl;
                } else {
                    std::cout << "回调请求网络错误: " << result.error().message << std::endl;
                }
            });
        
        // 等待回调完成
        co_await net::steady_timer(io_context, std::chrono::seconds(2)).async_wait(net::use_awaitable);
        
        std::cout << "\n=== 示例完成 ===" << std::endl;
        
        // 清理资源
        client->shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "示例执行错误: " << e.what() << std::endl;
    }
}
