//
// WSClientConfig.h
//
// author: Ruan Huipeng
// date : 2025-03-23
//

#ifndef ROC_NET_WSCLIENTCONFIG_H
#define ROC_NET_WSCLIENTCONFIG_H

#include <string>
#include <map>
#include <memory>

namespace roc::base::net {

// 前向声明
class INetworkConfig;

/**
 * @brief WebSocket 客户端配置类
 * 用于配置 WebSocket 连接的各种参数
 */
struct WSClientConfig {
    // 私有成员变量
private:
    std::string host_;
    std::string port_;
    uint32_t reconnect_interval_ = 5000; // 重连间隔，单位毫秒
    std::string path_ = "/ws"; // WebSocket 路径
    std::map<std::string, std::string> query_params_; // URL 查询参数
    std::map<std::string, std::string> headers_; // 请求头
    std::shared_ptr<INetworkConfig> network_config_; // 网络配置接口

public:
    // 构造函数
    WSClientConfig() = default;
    
    /**
     * @brief 构造函数
     * @param host 主机地址
     * @param port 端口号
     */
    WSClientConfig(const std::string& host, const std::string& port) 
        : host_(host), port_(port) {}
    
    // 设置方法
    /**
     * @brief 设置主机地址
     * @param host 主机地址
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& set_host(const std::string& host) {
        host_ = host;
        return *this;
    }
    
    /**
     * @brief 设置端口号
     * @param port 端口号
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& set_port(const std::string& port) {
        port_ = port;
        return *this;
    }
    
    /**
     * @brief 设置重连间隔
     * @param interval 重连间隔（毫秒）
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& set_reconnect_interval(uint32_t interval) {
        reconnect_interval_ = interval;
        return *this;
    }
    
    /**
     * @brief 设置WebSocket路径
     * @param path WebSocket路径
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& set_path(const std::string& path) {
        path_ = path;
        return *this;
    }
    
    /**
     * @brief 添加查询参数
     * @param key 参数名
     * @param value 参数值
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& add_query_param(const std::string& key, const std::string& value) {
        query_params_[key] = value;
        return *this;
    }
    
    /**
     * @brief 添加请求头
     * @param key 头名称
     * @param value 头值
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& add_header(const std::string& key, const std::string& value) {
        headers_[key] = value;
        return *this;
    }
    
    /**
     * @brief 设置网络配置
     * @param config 网络配置接口
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& set_network_config(std::shared_ptr<INetworkConfig> config) {
        network_config_ = config;
        return *this;
    }

    // 获取方法
    /**
     * @brief 获取主机地址
     * @return 主机地址
     */
    const std::string& get_host() const { return host_; }
    
    /**
     * @brief 获取端口号
     * @return 端口号
     */
    const std::string& get_port() const { return port_; }
    
    /**
     * @brief 获取重连间隔
     * @return 重连间隔（毫秒）
     */
    uint32_t get_reconnect_interval() const { return reconnect_interval_; }
    
    /**
     * @brief 获取WebSocket路径
     * @return WebSocket路径
     */
    const std::string& get_path() const { return path_; }
    
    /**
     * @brief 获取查询参数
     * @return 查询参数映射
     */
    const std::map<std::string, std::string>& get_query_params() const { return query_params_; }
    
    /**
     * @brief 获取请求头
     * @return 请求头映射
     */
    const std::map<std::string, std::string>& get_headers() const { return headers_; }
    
    /**
     * @brief 获取网络配置
     * @return 网络配置接口
     */
    std::shared_ptr<INetworkConfig> get_network_config() const { return network_config_; }
    
    /**
     * @brief 构建完整的 WebSocket URL
     * @return 完整的 WebSocket URL
     */
    std::string build_websocket_url() const {
        std::string url = "ws://" + host_ + ":" + port_ + path_;
        
        if (!query_params_.empty()) {
            url += "?";
            bool first = true;
            for (const auto& param : query_params_) {
                if (!first) url += "&";
                url += param.first + "=" + param.second;
                first = false;
            }
        }
        
        return url;
    }
    
    /**
     * @brief 构建目标路径（用于 WebSocket handshake）
     * @return 目标路径
     */
    std::string build_target() const {
        std::string target = path_;
        
        if (!query_params_.empty()) {
            target += "?";
            bool first = true;
            for (const auto& param : query_params_) {
                if (!first) target += "&";
                target += param.first + "=" + param.second;
                first = false;
            }
        }
        
        return target;
    }
    
    /**
     * @brief 重置配置到默认状态
     * @return 返回自身引用，支持链式调用
     */
    WSClientConfig& reset() {
        host_.clear();
        port_.clear();
        reconnect_interval_ = 5000;
        path_ = "/";
        query_params_.clear();
        headers_.clear();
        network_config_.reset();
        return *this;
    }
};

} // namespace roc::base::net

#endif // ROC_NET_WSCLIENTCONFIG_H

