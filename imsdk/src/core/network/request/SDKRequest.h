#pragma once

#include "base/network/include/LongConnectionClient.h"
#include "base/Uncopyable.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "base/network/include/Error.h"
#include "imsdk/src/core/network/request/SDKRequestEnum.h"

namespace roc::imsdk::network::request {

// ------------------------------------- Inter ----------------------------------------------
static std::atomic<int64_t> request_id_generator = 0;

// 发送消息
inline asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
send_message(SDKRoot *root, network::SendMessageReq *request);

// 拉取混链列表
inline asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
fetch_user_message_list(SDKRoot *root, network::FetchUserMessageListReq *request);

// 拉取单链
inline asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error>> 
fetch_conv_message_list(SDKRoot *root, network::FetchConvMessageListReq *request);

// ------------------------------------------------------------------------------------------



// ------------------------------------- Impl ------------------------------------------------
// 发送消息
inline asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
send_message(SDKRoot *root, network::SendMessageReq *request) {
    // 序列化request_body数据
    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::SEND_MESSAGE));
    req->set_data(request->SerializeAsString());
    req->set_requestid(std::to_string(request_id_generator++) + "_" + root->config().user_id + "_" + root->config().user_device_id);

    // 发送请求
    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await root->connection_manager()->send_request(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40201, "SDKRequest send_message response is empty"));
    }

    // 从返回数据中解析出 body
    auto resp = std::make_unique<network::SendMessageResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40201, "SDKRequest send_message parse response failed"));
    }

    co_return resp;
}

// 拉取混链列表
inline asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
fetch_user_message_list(SDKRoot *root, network::FetchUserMessageListReq *request) {
    // 序列化request_body数据
    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::FETCH_USER_MESSAGE_LIST));
    req->set_data(request->SerializeAsString());
    req->set_requestid(std::to_string(request_id_generator++) + "_" + root->config().user_id + "_" + root->config().user_device_id);

    // 发送请求
    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await root->connection_manager()->send_request(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list response is empty"));
    }

    // 从返回数据中解析出 body
    auto resp = std::make_unique<network::FetchUserMessageListResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list parse response failed"));
    }

    co_return resp;
}

// 拉取单链
inline asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error>> 
fetch_conv_message_list(SDKRoot *root, network::FetchConvMessageListReq *request) {
    // 序列化request_body数据
    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::FETCH_CONV_MESSAGE_LIST));
    req->set_data(request->SerializeAsString());
    req->set_requestid(std::to_string(request_id_generator++) + "_" + root->config().user_id + "_" + root->config().user_device_id);

    // 发送请求
    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await root->connection_manager()->send_request(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40202, "SDKRequest fetch_conv_message_list response is empty"));
    }

    // 从返回数据中解析出 body
    auto resp = std::make_unique<network::FetchConvMessageListResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40202, "SDKRequest fetch_conv_message_list parse response failed"));
    }

    co_return resp;
}

// ------------------------------------------------------------------------------------------


} // namespace roc::imsdk::network
