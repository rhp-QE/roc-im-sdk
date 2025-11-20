#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/core/common/macro.h"

#include <atomic>
#include <expected>
#include <string>

namespace roc::imsdk::network::request {

static std::atomic<int64_t> request_id_generator{0};

// 生成请求唯一 ID（内部工具方法）
static inline std::string next_request_id(SDKRoot *root) {
    return std::to_string(request_id_generator++) + "_" + root->config().user_id + "_" + root->config().user_device_id;
}

// 发送消息
asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
sendMessage(CONTEXT_T, network::SendMessageReq *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::SEND_MESSAGE));
    req->set_data(request->SerializeAsString());
    req->set_requestid(next_request_id(sdk_root.get()));
    req->set_trackid(TRACK_ID);

    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await sdk_root->ConnectionManager()->SendRequest(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40201, "SDKRequest send_message response is empty"));
    }

    auto resp = std::make_unique<network::SendMessageResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40201, "SDKRequest send_message parse response failed"));
    }

    co_return resp;
}

// 拉取混链列表
asio::awaitable<std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error>> 
fetchUserMessageList(CONTEXT_T, network::FetchUserMessageListReq *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::FETCH_USER_MESSAGE_LIST));
    req->set_data(request->SerializeAsString());
    req->set_requestid(next_request_id(sdk_root.get()));
    req->set_trackid(TRACK_ID);

    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await sdk_root->ConnectionManager()->SendRequest(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list response is empty"));
    }

    auto resp = std::make_unique<network::FetchUserMessageListResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list parse response failed"));
    }

    co_return resp;
}

// 拉取单链
asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error>> 
fetchConvMessageList(CONTEXT_T, network::FetchConvMessageListReq *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    std::unique_ptr<network::SdkWSReq> req = std::make_unique<network::SdkWSReq>();
    req->set_type(static_cast<int32_t>(SDKRequestType::FETCH_CONV_MESSAGE_LIST));
    req->set_data(request->SerializeAsString());
    req->set_requestid(next_request_id(sdk_root.get()));
    req->set_trackid(TRACK_ID);

    std::expected<std::unique_ptr<network::SdkWSResp>, roc::error::Error> response = co_await sdk_root->ConnectionManager()->SendRequest(req.get());
    if (!response || !response.has_value()) {
        co_return std::unexpected(roc::error::make_error(40202, "SDKRequest fetch_conv_message_list response is empty"));
    }

    auto resp = std::make_unique<network::FetchConvMessageListResp>();
    bool ok = resp->ParseFromArray(response.value()->data().data(), response.value()->data().size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40202, "SDKRequest fetch_conv_message_list parse response failed"));
    }

    co_return resp;
}

} // namespace roc::imsdk::network::request


