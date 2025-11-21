#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"

#include <boost/asio/awaitable.hpp>
#include <memory>
#include <expected>

namespace roc::imsdk::core::conversation {

class UserMessageFetcher {
public:
    explicit UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);

    /// 获取用户消息
    boost::asio::awaitable<void> FetchUserMessages(CONTEXT_T);
    
private:
    /// 构造获取用户消息列表请求
    std::unique_ptr<network::FetchUserMessageListReq> p_MakeFetchUserMessageListReq(CONTEXT_T, int64_t cursor);
    
    /// 处理获取到的用户消息
    boost::asio::awaitable<void> p_HandleFetchedUserMessage(CONTEXT_T, std::unique_ptr<network::FetchUserMessageListResp> resp);

    /// 发送获取用户消息列表请求
    boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
        p_Request(CONTEXT_T, network::FetchUserMessageListReq *request);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::conversation