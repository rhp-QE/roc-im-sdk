//
// SDKConnectionManager.cc
//
// IM SDK 连接管理器实现
//
// author: AI Assistant
// date: 2025-01-xx
//

#include "SDKConnectionManager.h"
#include "base/network/include/LongConnectionClient.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/json.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <memory>
#include <mutex>
#include <string>
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "base/utils/utils.h"

namespace json = boost::json;
namespace asio = boost::asio;

namespace roc::imsdk::network {

//--------------------
base::net::LongConnectionConfig generateNetConfig(roc::imsdk::SDKRoot* root);
//--------------------

SDKConnectionManager::SDKConnectionManager(boost::asio::io_context &io_context) : net_io_context_(io_context) {}

boost::asio::awaitable<void> SDKConnectionManager::init_and_connect(std::weak_ptr<SDKRoot> root) {

    root_ = root;

    lc_ = std::make_unique<base::net::LongConnectionClient>(generateNetConfig(root.lock().get()), net_io_context_);

    // 观察网络状态变更
    lc_->set_connection_status_callback([](bool connected, const std::string &detail) {
        std::cout << "SDKConnectionManager::connection_status: " << connected << std::endl;
    });

    // 
    lc_->set_data_received_callback([wroot = root_](boost::beast::flat_buffer data) {
        std::shared_ptr<SDKRoot> sroot = wroot.lock();
        if (!sroot) {
            return;
        }

        std::cout<<"[rhpmark] sdk receive message"<<std::endl;

        auto conn = sroot->connection_manager();
        boost::asio::co_spawn(conn->net_io_context_, conn->handle_data_received(std::move(data)), asio::detached);
    });

    auto res = co_await lc_->connect();

    co_return;
}

void SDKConnectionManager::set_on_push_message_callback(OnPushMesageCallbackType callback) {
    on_push_message_callbacks.push_back(callback);
}

void SDKConnectionManager::all_component_did_load() {
    // 组件加载完成后的初始化逻辑
}


boost::asio::awaitable<std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error>> SDKConnectionManager::send_request(network::SdkWSReq *req) {
    std::shared_ptr<SDKRoot> root = root_.lock();
    if (!root) {
        co_return std::unexpected(roc::error::make_error(1000, "root is expired", "SDKConnectionManager:send_request"));
    }

    if (req->requestid().empty()) {
        co_return std::unexpected(roc::error::make_error(1000, "requestid is empty", "SDKConnectionManager:send_request"));
    }

    auto channel = std::make_shared<channel_type>(net_io_context_, 1);
    std::string request_id_str = req->requestid();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map[request_id_str] = channel;
    }

    std::vector<char> buffer(req->ByteSizeLong());
    req->SerializeToArray(buffer.data(), req->ByteSizeLong());
    auto result = co_await lc_->send_data(buffer.data(), buffer.size());

    std::unique_ptr<network::SdkWSResp> resp = co_await channel->async_receive(boost::asio::use_awaitable);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        channel_map.erase(request_id_str);
    }

    co_return std::move(resp);
}

boost::asio::awaitable<void> SDKConnectionManager::handle_data_received(boost::beast::flat_buffer data) {
    try {
        std::unique_ptr<network::SdkWSResp> resp = std::make_unique<network::SdkWSResp>();
        resp->ParseFromArray(data.data().data(), data.size());
        const std::string &request_id = resp->requestid();
        std::shared_ptr<channel_type> channel;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto iter = channel_map.find(request_id);
            if (iter != channel_map.end()) {
                channel = iter->second;
                channel_map.erase(iter);
            }
        }

        if (!channel) {
            /// 直接转发给所有消息者消费， 自己进行数据解析
            std::shared_ptr<network::SdkWSResp> s_resp = std::move(resp);
            for (const auto &callback : on_push_message_callbacks) {
                base::util::safe_invoke_block(callback, s_resp);
            }
        } else {
            // 唤醒请求携程
            co_await channel->async_send(boost::system::error_code{}, std::move(resp), boost::asio::use_awaitable);
        }

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    co_return;
}



//--------------- no member private method ----------------------
base::net::LongConnectionConfig generateNetConfig(roc::imsdk::SDKRoot* root) {
    roc::base::net::LongConnectionConfig config("127.0.0.1", "10010");
    config.set_heartbeat_interval(5000)
    .set_heartbeat_timeout(10000)
    .set_heartbeat_payload("ping")
    .set_auto_reconnect(true)
    .set_max_reconnect_attempts(5)
    .set_reconnect_backoff(1000)
    .add_header("User-Agent", "LongConnectionClient/1.0")
    .add_query_param("sendID", root->config().user_id)
    .add_query_param("sdkType", "rocSDK-c++");

    return config;
}
//---------------------------------------------------------------

} // namespace roc::imsdk::network 