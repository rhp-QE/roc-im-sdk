#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>
#include <chrono>
#include <memory>
#include <expected>
#include <functional>
#include <boost/asio.hpp>
#include <boost/json.hpp>

namespace roc::network {

namespace net = boost::asio;
using json = boost::json::value;

// HTTP 方法枚举
enum class HttpMethod {
    M_GET, M_POST, M_PUT, M_DELETE, M_PATCH, M_HEAD, M_OPTIONS
};

// HTTP 错误类型
enum class HttpErrorCode {
    NETWORK_ERROR, TIMEOUT, INVALID_URL, SSL_ERROR, CANCELLED
};

// HTTP 错误结构
struct HttpError {
    HttpErrorCode code;
    std::string message;
    
    static HttpError network_error(std::string_view msg) {
        return {HttpErrorCode::NETWORK_ERROR, std::string(msg)};
    }
    
    static HttpError timeout_error(std::string_view msg) {
        return {HttpErrorCode::TIMEOUT, std::string(msg)};
    }
    
    static HttpError invalid_url(std::string_view msg) {
        return {HttpErrorCode::INVALID_URL, std::string(msg)};
    }
};

// HTTP 响应类
class HttpResponse {
public:
    // 获取状态码
    int status_code() const noexcept { return status_code_; }
    
    // 获取状态消息
    std::string_view status_message() const noexcept { return status_message_; }
    
    // 获取响应头
    std::optional<std::string_view> header(std::string_view key) const noexcept;
    const std::unordered_map<std::string, std::string>& headers() const noexcept { return headers_; }
    
    // 获取响应体
    std::string_view text() const { return body_; }
    json to_json() const;
    std::vector<uint8_t> bytes() const;
    
    // 检查状态码类别
    bool is_success() const noexcept { return status_code_ >= 200 && status_code_ < 300; }
    bool is_redirect() const noexcept { return status_code_ >= 300 && status_code_ < 400; }
    bool is_client_error() const noexcept { return status_code_ >= 400 && status_code_ < 500; }
    bool is_server_error() const noexcept { return status_code_ >= 500 && status_code_ < 600; }
    
    // 获取原始响应大小信息
    size_t content_length() const noexcept { return body_.size(); }
    
private:
    int status_code_{0};
    std::string status_message_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    
    friend class HttpClient;
};

// HTTP 请求构建器
class HttpRequestBuilder {
public:
    // 设置 URL
    HttpRequestBuilder& url(std::string_view url);
    
    // 设置 HTTP 方法
    HttpRequestBuilder& method(HttpMethod method);
    
    // 添加请求头
    HttpRequestBuilder& header(std::string_view key, std::string_view value);
    
    // 设置查询参数
    HttpRequestBuilder& query_param(std::string_view key, std::string_view value);
    
    // 设置请求体 (支持多种类型)
    HttpRequestBuilder& body(std::string_view content);
    HttpRequestBuilder& body(const json& json_data);
    HttpRequestBuilder& body(std::vector<uint8_t> binary_data);
    
    // 设置超时
    HttpRequestBuilder& timeout(std::chrono::milliseconds ms);
    
    // 设置认证信息
    HttpRequestBuilder& basic_auth(std::string_view username, std::string_view password);
    HttpRequestBuilder& bearer_token(std::string_view token);
    
    // 执行请求
    net::awaitable<std::expected<HttpResponse, HttpError>> execute();
    void execute(std::function<void(std::expected<HttpResponse, HttpError>)> callback);
    
private:
    std::string url_;
    HttpMethod method_{HttpMethod::M_GET};
    std::unordered_map<std::string, std::string> headers_;
    std::unordered_map<std::string, std::string> query_params_;
    std::string body_;
    std::chrono::milliseconds timeout_{30000};
    class HttpClient* client_{nullptr};
    
    std::string build_url() const;
    friend class HttpClient;
};

// HTTP 客户端配置
struct HttpClientConfig {
    std::chrono::milliseconds default_timeout{30000};
    bool verify_ssl_certificates{true};
    std::string user_agent{"ROC-HttpClient/1.0"};
    
    // 链式配置
    HttpClientConfig& timeout(std::chrono::milliseconds ms) { default_timeout = ms; return *this; }
    HttpClientConfig& ssl_verification(bool verify) { verify_ssl_certificates = verify; return *this; }
    HttpClientConfig& user_agent_string(std::string_view agent) { user_agent = agent; return *this; }
};

// HTTP 客户端类
class HttpClient {
public:
    explicit HttpClient(net::io_context& io_context, HttpClientConfig config = {});
    ~HttpClient();
    
    // 禁用复制，允许移动
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) noexcept;
    HttpClient& operator=(HttpClient&&) noexcept;
    
    // 便利方法 - 创建请求构建器
    HttpRequestBuilder get(std::string_view url);
    HttpRequestBuilder post(std::string_view url);
    HttpRequestBuilder put(std::string_view url);
    HttpRequestBuilder delete_request(std::string_view url);
    HttpRequestBuilder patch(std::string_view url);
    HttpRequestBuilder head(std::string_view url);
    HttpRequestBuilder options(std::string_view url);
    
    // 直接执行请求的便利方法
    net::awaitable<std::expected<HttpResponse, HttpError>> 
    get_json(std::string_view url, const json& params = json{});
    
    net::awaitable<std::expected<HttpResponse, HttpError>> 
    post_json(std::string_view url, const json& data);
    
    net::awaitable<std::expected<HttpResponse, HttpError>> 
    put_json(std::string_view url, const json& data);
    
    // 配置管理
    void update_config(const HttpClientConfig& config);
    const HttpClientConfig& get_config() const noexcept;
    
    // 资源管理
    void shutdown();
    bool is_shutdown() const noexcept;
    
private:
    class HttpClientImpl;
    std::unique_ptr<HttpClientImpl> impl_;
    
    // 内部执行方法
    net::awaitable<std::expected<HttpResponse, HttpError>>
    execute_request(const HttpRequestBuilder& builder);
    
    void execute_request_callback(const HttpRequestBuilder& builder,
                                 std::function<void(std::expected<HttpResponse, HttpError>)> callback);
    
    friend class HttpRequestBuilder;
};

// 工厂函数
std::unique_ptr<HttpClient> create_http_client(
    net::io_context& io_context, 
    HttpClientConfig config = {});

// 辅助函数
std::string http_method_to_string(HttpMethod method);
std::string url_encode(std::string_view str);
std::string base64_encode(std::string_view str);

} // namespace roc::network 