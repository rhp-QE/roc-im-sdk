#pragma once

#include "base/network/include/LongConnectionClient.h"
#include "base/Uncopyable.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "base/network/include/Error.h"
#include "imsdk/src/core/network/request/SDKRequestEnum.h"

namespace roc::imsdk::network::request {

// ------------------------------------- Inter ----------------------------------------------

// 发送消息
asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
    send_message(SDKRoot *root, network::SendMessageReq *request);

// 拉取混链列表
asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
    fetch_user_message_list(SDKRoot *root, network::FetchUserMessageListReq *request);

// 拉取单链
asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error>> 
    fetch_conv_message_list(SDKRoot *root, network::FetchConvMessageListReq *request);

// ------------------------------------------------------------------------------------------



// ------------------------------------- Impl ------------------------------------------------
// 实现在 SDKRequest.cc 中
// ------------------------------------------------------------------------------------------


} // namespace roc::imsdk::network
