#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"

#include <atomic>
#include <boost/asio/awaitable.hpp>
#include <memory>
#include <expected>

namespace roc::imsdk::core::conversation {

class UserMessageFetcher {
public:
    explicit UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);

    /// 获取用户消息
    boost::asio::awaitable<void> FetchUserMessages(CTX_T);
    
private:
    /// 构造获取用户消息列表请求
    std::unique_ptr<network::FetchUserMessageListReq> p_makeFetchUserMessageListReq(CTX_T, bool news, int64_t cursor, bool forward);
    
    /// 处理获取到的用户消息
    boost::asio::awaitable<void> p_handleFetchedUserMessage(CTX_T, std::vector<std::shared_ptr<network::ConversationInfo>> net_convs);

    /// 发送获取用户消息列表请求
    boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
        p_request(CTX_T, network::FetchUserMessageListReq *request);

    /// 二次校验混链拉取的完整性
    boost::asio::awaitable<bool> p_doubleCheckUserMessageIntegrity(CTX_T, int64_t left, int64_t right);

    /// 是否正在拉取混链
    std::atomic<bool> is_pulling;

    /// 混链拉下来的会话
    std::vector<std::string> pulled_convs_;

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation