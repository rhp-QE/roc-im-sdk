#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::core::conversation {

class UserMessageFetcher {
public:
    /// 获取用户消息
    static boost::asio::awaitable<void> FetchUserMessages(CONTEXT_T);
    
private:
    /// 构造获取用户消息列表请求
    static std::unique_ptr<network::FetchUserMessageListReq> p_MakeFetchUserMessageListReq(CONTEXT_T, int64_t cursor);
    
    /// 处理获取到的用户消息
    static boost::asio::awaitable<void> p_HandleFetchedUserMessage(CONTEXT_T, std::unique_ptr<network::FetchUserMessageListResp> resp);
};

} // namespace roc::imsdk::core::conversation