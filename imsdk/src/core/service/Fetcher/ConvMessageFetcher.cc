///
/// @file   ConvMessageFetcher.cc
/// @brief  会话消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/ConvMessageFetcher.h"
#include "imsdk/src/core/macro.h"

namespace roc::imsdk::service {

// private function declare ----------------------------------------------------------
std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(SDKRoot *root);
// ----------------------------------------------------------------------------------

ConvMessageFetcher::ConvMessageFetcher(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {
}

ConvMessageFetcher::~ConvMessageFetcher() = default;

asio::awaitable<void> ConvMessageFetcher::fetch_conv_message_list() {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root_);

}









// private function impl ------------------------------------------------------------

std::unique_ptr<network::FetchConvMessageListReq> p_make_fetch_conv_message_list_req(SDKRoot *root) {
    auto req = std::make_unique<network::FetchConvMessageListReq>();
    return req;
}
// ----------------------------------------------------------------------------------

} // namespace roc::imsdk::service