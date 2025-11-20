#pragma once

#include "imsdk/base/include/network/LongConnectionClient.h"
#include "imsdk/base/include/uncopyable.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/core/network/request/SDKRequestEnum.h"

namespace roc::imsdk::network::request {

// ------------------------------------- Inter ----------------------------------------------

// 发送消息
asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
    sendMessage(CONTEXT_T, network::SendMessageReq *request);

// 拉取混链列表
asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
    fetchUserMessageList(CONTEXT_T, network::FetchUserMessageListReq *request);

// 拉取单链
asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error>> 
    fetchConvMessageList(CONTEXT_T, network::FetchConvMessageListReq *request);

// ------------------------------------------------------------------------------------------



// ------------------------------------- Impl ------------------------------------------------
// 实现在 SDKRequest.cc 中
// ------------------------------------------------------------------------------------------


} // namespace roc::imsdk::network
