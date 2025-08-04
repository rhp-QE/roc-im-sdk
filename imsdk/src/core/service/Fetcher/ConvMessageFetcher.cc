///
/// @file   ConvMessageFetcher.cc
/// @brief  会话消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/ConvMessageFetcher.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/request/SDKRequest.h"

namespace roc::imsdk::service {

// private function declare ----------------------------------------------------------
std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(SDKRoot *root);
// ----------------------------------------------------------------------------------

ConvMessageFetcher::ConvMessageFetcher(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {
}

ConvMessageFetcher::~ConvMessageFetcher() = default;

asio::awaitable<void> ConvMessageFetcher::fetch_conv_message_list() {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root_);

    // 构造请求
    std::unique_ptr<network::FetchConvMessageListReq> req = p_make_fetch_conv_message_list_req(sdk_root.get());

    // 发送请求
    std::expected<std::unique_ptr<network::FetchConvMessageListResp>, roc::error::Error> resp = co_await network::request::fetch_conv_message_list(sdk_root.get(), req.get());

}









// private function impl ------------------------------------------------------------

std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(SDKRoot *root) {
    auto req = std::make_unique<network::FetchConvMessageListReq>();

    return req;
}
// ----------------------------------------------------------------------------------

} // namespace roc::imsdk::service