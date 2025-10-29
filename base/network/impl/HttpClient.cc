#include "base/network/include/HttpClient.h"
#include <boost/beast.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <regex>
#include <sstream>
#include <iomanip>

namespace roc::network {

namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = net::ssl;

// 简洁的内部实现类
class HttpClient::HttpClientImpl {
public:
    HttpClientImpl(net::io_context& io_context, HttpClientConfig config)
        : io_context_(io_context)
        , ssl_context_(ssl::context::tlsv12_client)
        , config_(std::move(config))
        , shutdown_(false) {
        
        // 基本SSL配置
        ssl_context_.set_default_verify_paths();
        ssl_context_.set_verify_mode(config_.verify_ssl_certificates ? 
                                   ssl::verify_peer : ssl::verify_none);
    }
    
    net::awaitable<std::expected<HttpResponse, HttpError>>
    execute_request(const HttpRequestBuilder& builder) {
        if (shutdown_) {
            co_return std::unexpected(HttpError{HttpErrorCode::CANCELLED, "Client shutdown"});
        }
        
        try {
            // 解析URL
            auto url_parts = parse_url(builder.build_url());
            if (!url_parts) {
                co_return std::unexpected(url_parts.error());
            }
            
            auto [host, port, path, is_ssl] = url_parts.value();
            
            // 创建HTTP请求
            auto http_req = build_http_request(builder, host, path);
            
            // 发送请求并获取响应
            auto beast_response = co_await send_request(http_req, host, port, is_ssl, builder.timeout_);
            if (!beast_response) {
                co_return std::unexpected(beast_response.error());
            }
            
            // 转换为我们的响应格式
            HttpResponse response;
            response.status_code_ = beast_response->result_int();
            response.status_message_ = std::string(beast_response->reason());
            response.body_ = beast_response->body();
            
            // 转换头部
            for (const auto& field : *beast_response) {
                response.headers_[std::string(field.name_string())] = std::string(field.value());
            }
            
            co_return response;
            
        } catch (const std::exception& e) {
            co_return std::unexpected(HttpError::network_error(e.what()));
        }
    }
    
    void execute_request_callback(const HttpRequestBuilder& builder,
                                 std::function<void(std::expected<HttpResponse, HttpError>)> callback) {
        net::co_spawn(io_context_,
            [this, builder, callback = std::move(callback)]() -> net::awaitable<void> {
                auto result = co_await execute_request(builder);
                callback(std::move(result));
            }, net::detached);
    }
    
    void shutdown() { shutdown_ = true; }
    bool is_shutdown() const { return shutdown_; }
    void update_config(const HttpClientConfig& config) { config_ = config; }
    const HttpClientConfig& get_config() const { return config_; }
    
private:
    net::io_context& io_context_;
    ssl::context ssl_context_;
    HttpClientConfig config_;
    std::atomic<bool> shutdown_;
    
    // URL解析结果
    struct UrlParts {
        std::string host, port, path;
        bool is_ssl;
    };
    
    std::expected<UrlParts, HttpError> parse_url(const std::string& url) {
        std::regex url_regex(R"(^(https?)://([^:/\s]+)(?::(\d+))?([^?\s]*)?(?:\?([^\s]*))?$)");
        std::smatch match;
        
        if (!std::regex_match(url, match, url_regex)) {
            return std::unexpected(HttpError::invalid_url("Invalid URL: " + url));
        }
        
        std::string scheme = match[1].str();
        std::string host = match[2].str();
        std::string port = match[3].str();
        std::string path = match[4].str();
        std::string query = match[5].str();
        
        bool is_ssl = (scheme == "https");
        if (port.empty()) port = is_ssl ? "443" : "80";
        if (path.empty()) path = "/";
        if (!query.empty()) path += "?" + query;
        
        return UrlParts{host, port, path, is_ssl};
    }
    
    http::request<http::string_body> build_http_request(const HttpRequestBuilder& builder,
                                                       const std::string& host,
                                                       const std::string& path) {
        // 转换HTTP方法
        http::verb verb = http::verb::get;
        switch (builder.method_) {
            case HttpMethod::M_GET: verb = http::verb::get; break;
            case HttpMethod::M_POST: verb = http::verb::post; break;
            case HttpMethod::M_PUT: verb = http::verb::put; break;
            case HttpMethod::M_DELETE: verb = http::verb::delete_; break;
            case HttpMethod::M_PATCH: verb = http::verb::patch; break;
            case HttpMethod::M_HEAD: verb = http::verb::head; break;
            case HttpMethod::M_OPTIONS: verb = http::verb::options; break;
        }
        
        http::request<http::string_body> req{verb, path, 11};
        
        // 设置基本头部
        req.set(http::field::host, host);
        req.set(http::field::user_agent, config_.user_agent);
        
        // 设置自定义头部
        for (const auto& [key, value] : builder.headers_) {
            req.set(key, value);
        }
        
        // 设置请求体
        if (!builder.body_.empty()) {
            req.body() = builder.body_;
            req.prepare_payload();
        }
        
        return req;
    }
    
    net::awaitable<std::expected<http::response<http::string_body>, HttpError>>
    send_request(const http::request<http::string_body>& req,
                const std::string& host, const std::string& port,
                bool is_ssl, std::chrono::milliseconds timeout) {
        
        if (is_ssl) {
            co_return co_await send_https_request(req, host, port, timeout);
        } else {
            co_return co_await send_http_request(req, host, port, timeout);
        }
    }
    
    net::awaitable<std::expected<http::response<http::string_body>, HttpError>>
    send_http_request(const http::request<http::string_body>& req,
                     const std::string& host, const std::string& port,
                     std::chrono::milliseconds timeout) {
        try {
            beast::tcp_stream stream(io_context_);
            stream.expires_after(timeout);
            
            // 连接
            auto resolver = net::ip::tcp::resolver(io_context_);
            auto endpoints = co_await resolver.async_resolve(host, port, net::use_awaitable);
            co_await stream.async_connect(endpoints, net::use_awaitable);
            
            // 发送请求
            co_await http::async_write(stream, req, net::use_awaitable);
            
            // 接收响应
            beast::flat_buffer buffer;
            http::response<http::string_body> response;
            co_await http::async_read(stream, buffer, response, net::use_awaitable);
            
            // 关闭连接
            beast::error_code ec;
            stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);
            
            co_return response;
            
        } catch (const std::exception& e) {
            co_return std::unexpected(HttpError::network_error(e.what()));
        }
    }
    
    net::awaitable<std::expected<http::response<http::string_body>, HttpError>>
    send_https_request(const http::request<http::string_body>& req,
                      const std::string& host, const std::string& port,
                      std::chrono::milliseconds timeout) {
        try {
            beast::ssl_stream<beast::tcp_stream> stream(io_context_, ssl_context_);
            beast::get_lowest_layer(stream).expires_after(timeout);
            
            // 连接
            auto resolver = net::ip::tcp::resolver(io_context_);
            auto endpoints = co_await resolver.async_resolve(host, port, net::use_awaitable);
            co_await beast::get_lowest_layer(stream).async_connect(endpoints, net::use_awaitable);
            
            // SSL握手
            co_await stream.async_handshake(ssl::stream_base::client, net::use_awaitable);
            
            // 发送请求
            co_await http::async_write(stream, req, net::use_awaitable);
            
            // 接收响应
            beast::flat_buffer buffer;
            http::response<http::string_body> response;
            co_await http::async_read(stream, buffer, response, net::use_awaitable);
            
            // 关闭连接
            beast::error_code ec;
            beast::get_lowest_layer(stream).socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);
            
            co_return response;
            
        } catch (const std::exception& e) {
            co_return std::unexpected(HttpError::network_error(e.what()));
        }
    }
};

// HttpResponse 实现
std::optional<std::string_view> HttpResponse::header(std::string_view key) const noexcept {
    auto it = headers_.find(std::string(key));
    return (it != headers_.end()) ? std::optional<std::string_view>(it->second) : std::nullopt;
}

json HttpResponse::to_json() const {
    try {
        return boost::json::parse(body_);
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to parse response as JSON");
    }
}

std::vector<uint8_t> HttpResponse::bytes() const {
    return std::vector<uint8_t>(body_.begin(), body_.end());
}

// HttpRequestBuilder 实现
HttpRequestBuilder& HttpRequestBuilder::url(std::string_view url) {
    url_ = url;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::method(HttpMethod method) {
    method_ = method;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::header(std::string_view key, std::string_view value) {
    headers_[std::string(key)] = std::string(value);
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::query_param(std::string_view key, std::string_view value) {
    query_params_[std::string(key)] = std::string(value);
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::body(std::string_view content) {
    body_ = content;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::body(const json& json_data) {
    body_ = boost::json::serialize(json_data);
    header("Content-Type", "application/json");
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::body(std::vector<uint8_t> binary_data) {
    body_ = std::string(binary_data.begin(), binary_data.end());
    header("Content-Type", "application/octet-stream");
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::timeout(std::chrono::milliseconds ms) {
    timeout_ = ms;
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::basic_auth(std::string_view username, std::string_view password) {
    std::string credentials = std::string(username) + ":" + std::string(password);
    header("Authorization", "Basic " + base64_encode(credentials));
    return *this;
}

HttpRequestBuilder& HttpRequestBuilder::bearer_token(std::string_view token) {
    header("Authorization", "Bearer " + std::string(token));
    return *this;
}

std::string HttpRequestBuilder::build_url() const {
    std::string result = url_;
    
    if (!query_params_.empty()) {
        result += (result.find('?') != std::string::npos) ? "&" : "?";
        bool first = true;
        for (const auto& [key, value] : query_params_) {
            if (!first) result += "&";
            result += url_encode(key) + "=" + url_encode(value);
            first = false;
        }
    }
    
    return result;
}

net::awaitable<std::expected<HttpResponse, HttpError>> HttpRequestBuilder::execute() {
    if (!client_) {
        co_return std::unexpected(HttpError::network_error("No client associated"));
    }
    co_return co_await client_->execute_request(*this);
}

void HttpRequestBuilder::execute(std::function<void(std::expected<HttpResponse, HttpError>)> callback) {
    if (!client_) {
        callback(std::unexpected(HttpError::network_error("No client associated")));
        return;
    }
    client_->execute_request_callback(*this, std::move(callback));
}

// HttpClient 实现
HttpClient::HttpClient(net::io_context& io_context, HttpClientConfig config)
    : impl_(std::make_unique<HttpClientImpl>(io_context, std::move(config))) {
}

HttpClient::~HttpClient() = default;
HttpClient::HttpClient(HttpClient&&) noexcept = default;
HttpClient& HttpClient::operator=(HttpClient&&) noexcept = default;

HttpRequestBuilder HttpClient::get(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_GET);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::post(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_POST);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::put(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_PUT);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::delete_request(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_DELETE);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::patch(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_PATCH);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::head(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_HEAD);
    builder.client_ = this;
    return builder;
}

HttpRequestBuilder HttpClient::options(std::string_view url) {
    HttpRequestBuilder builder;
    builder.url(url).method(HttpMethod::M_OPTIONS);
    builder.client_ = this;
    return builder;
}

net::awaitable<std::expected<HttpResponse, HttpError>> 
HttpClient::get_json(std::string_view url, const json& params) {
    auto builder = get(url);
    
    // 将JSON参数转换为查询参数
    if (params.is_object()) {
        for (const auto& [key, value] : params.as_object()) {
            if (value.is_string()) {
                builder.query_param(key, value.as_string().c_str());
            } else if (value.is_number()) {
                builder.query_param(key, std::to_string(value.as_double()));
            } else if (value.is_bool()) {
                builder.query_param(key, value.as_bool() ? "true" : "false");
            }
        }
    }
    
    co_return co_await builder.execute();
}

net::awaitable<std::expected<HttpResponse, HttpError>> 
HttpClient::post_json(std::string_view url, const json& data) {
    co_return co_await post(url).body(data).execute();
}

net::awaitable<std::expected<HttpResponse, HttpError>> 
HttpClient::put_json(std::string_view url, const json& data) {
    co_return co_await put(url).body(data).execute();
}

void HttpClient::update_config(const HttpClientConfig& config) {
    impl_->update_config(config);
}

const HttpClientConfig& HttpClient::get_config() const noexcept {
    return impl_->get_config();
}

void HttpClient::shutdown() {
    impl_->shutdown();
}

bool HttpClient::is_shutdown() const noexcept {
    return impl_->is_shutdown();
}

net::awaitable<std::expected<HttpResponse, HttpError>>
HttpClient::execute_request(const HttpRequestBuilder& builder) {
    co_return co_await impl_->execute_request(builder);
}

void HttpClient::execute_request_callback(const HttpRequestBuilder& builder,
                                         std::function<void(std::expected<HttpResponse, HttpError>)> callback) {
    impl_->execute_request_callback(builder, std::move(callback));
}

// 工厂函数
std::unique_ptr<HttpClient> create_http_client(net::io_context& io_context, HttpClientConfig config) {
    return std::make_unique<HttpClient>(io_context, std::move(config));
}

// 辅助函数
std::string http_method_to_string(HttpMethod method) {
    switch (method) {
        case HttpMethod::M_GET: return "GET";
        case HttpMethod::M_POST: return "POST";
        case HttpMethod::M_PUT: return "PUT";
        case HttpMethod::M_DELETE: return "DELETE";
        case HttpMethod::M_PATCH: return "PATCH";
        case HttpMethod::M_HEAD: return "HEAD";
        case HttpMethod::M_OPTIONS: return "OPTIONS";
        default: return "GET";
    }
}

std::string url_encode(std::string_view str) {
    std::ostringstream encoded;
    encoded << std::hex << std::uppercase;
    
    for (char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<unsigned char>(c);
        }
    }
    
    return encoded.str();
}

std::string base64_encode(std::string_view str) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    
    int val = 0, valb = -6;
    for (unsigned char c : str) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    
    return encoded;
}

} // namespace roc::network 