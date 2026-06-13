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

#include "imsdk/base/include/network/LongConnectionClient.h"
#include "imsdk/base/include/uncopyable.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/connection/request/RequestTracker.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/network.h"

#include <boost/asio/io_context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <expected>
#include <boost/asio.hpp>
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
using OnPushMesageCallbackType = std::function<void(std::shared_ptr<const network::FrontierMessage>)>;
using OnConnectionStatusChangeCallbackType = std::function<void(roc::imsdk::network::NetworkStatus)>;

// ------------------------------------------------------------------------------------------
// 职责：1、长连接的初始化，链接的管理
//      2、request、response 的关联管理
// ------------------------------------------------------------------------------------------
class SDKConnectionManager : public std::enable_shared_from_this<SDKConnectionManager>,
                             public roc::base::uncopyable {
public:

    SDKConnectionManager(std::shared_ptr<SDKRoot> sdk_root);

    boost::asio::awaitable<bool> InitAndConnect(std::shared_ptr<SDKRoot> sdk_root);

    boost::asio::awaitable<bool> disconnect();

    // 组件加载完成后的初始化
    void AllComponentDidLoad(CTX_T);

    // 添加接收消息回调
    void AddOnPushMessageCallback(OnPushMesageCallbackType callback);

    // 发送请求
    boost::asio::awaitable<std::expected<std::unique_ptr<FrontierMessage>, roc::error::Error>> SendRequest(CTX_T, std::unique_ptr<FrontierMessage> req);

    /// 获取网络状态
    roc::imsdk::network::NetworkStatus GetNetworkStatus();

    /// 设置网络状态变更回调
    void OnNetworkStatusChange(OnConnectionStatusChangeCallbackType callback);

private:
    std::vector<OnConnectionStatusChangeCallbackType> network_status_change_callback_;

    std::unique_ptr<base::net::LongConnectionClient> lc_;
    std::unique_ptr<RequestTracker> request_tracker_;
    std::mutex mutex_;

    std::vector<OnPushMesageCallbackType> on_push_message_callbacks_;

    std::weak_ptr<SDKRoot> w_sdk_root;

    boost::asio::awaitable<void> handleDataReceived(boost::beast::flat_buffer data);

    boost::asio::awaitable<void> handleMessageFrame(std::string frame);

    base::net::LongConnectionConfig p_GenerateNetConfig(roc::imsdk::SDKRoot* root);

};
}
#endif // ROC_IM_SDK_CONNECTION_MANAGER_H
