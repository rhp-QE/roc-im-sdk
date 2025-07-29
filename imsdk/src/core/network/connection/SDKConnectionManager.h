//
// SDKConnectionManager.h
//
// IM SDK 连接管理器
// 负责管理 SDK 的长连接，底层使用 LongConnectionClient
//
// author: AI Assistant
// date: 2025-01-xx
//

#ifndef ROC_IM_SDK_CONNECTION_MANAGER_H
#define ROC_IM_SDK_CONNECTION_MANAGER_H

#include "base/network/include/LongConnectionClient.h"
#include "base/Uncopyable.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <expected>
#include <boost/asio.hpp>
#include <unordered_map>
#include <boost/asio/use_awaitable.hpp>

namespace roc::imsdk {
    class SDKRoot;
}

namespace roc::imsdk::network {

// 前向声明
namespace error {
    class Error;
}


// call back
using OnPushMesageCallbackType = std::function<void(std::unique_ptr<network::SdkWSResp>)> ;

// ------------------------------------------------------------------------------------------
// 职责：1、长连接的初始化，链接的管理
//      2、request、response 的关联管理
// ------------------------------------------------------------------------------------------
class SDKConnectionManager : public std::enable_shared_from_this<SDKConnectionManager>,
                             public roc::base::uncopyable {
public:

    SDKConnectionManager(boost::asio::io_context &io_context);

    boost::asio::awaitable<void> init_and_connect(std::weak_ptr<SDKRoot> root);

    void set_on_push_message_callback(OnPushMesageCallbackType callback);

    boost::asio::awaitable<std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error>> send_request(network::SdkWSReq *req);

private:
    std::unique_ptr<base::net::LongConnectionClient> lc_;
    
    std::atomic_uint64_t request_id = 0;
    using channel_type = boost::asio::experimental::channel<void(boost::system::error_code, std::unique_ptr<network::SdkWSResp>)>;
    std::unordered_map<std::string, std::shared_ptr<channel_type>> channel_map;
    boost::asio::io_context &net_io_context_;
    std::mutex mutex_;

    std::weak_ptr<SDKRoot> root_;

    boost::asio::awaitable<void> handle_data_received(boost::beast::flat_buffer data);
};
}
#endif // ROC_IM_SDK_CONNECTION_MANAGER_H 