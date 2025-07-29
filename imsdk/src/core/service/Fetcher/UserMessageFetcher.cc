///
/// @file   UserMessageFetcher.cc
/// @brief  用户消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::service {

// private function declare ----------------------------------------------------------
std::unique_ptr<network::FetchUserMessageListReq> p_make_fetch_user_message_list_req(SDKRoot *root, int64_t cursor);
boost::asio::awaitable<void> handle_fetched_user_message(std::weak_ptr<SDKRoot> w_root, std::unique_ptr<network::FetchUserMessageListResp> resp);
// ----------------------------------------------------------------------------------

UserMessageFetcher::UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root) : sdk_root_(sdk_root) {
}

UserMessageFetcher::~UserMessageFetcher() = default;

asio::awaitable<bool> UserMessageFetcher::fetch_user_messages() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(sdk_root_, false);

    // 获取最新游标
    int64_t cursor = 0;

    // 构造请求
    std::unique_ptr<network::FetchUserMessageListReq> req = p_make_fetch_user_message_list_req(sdk_root.get(), cursor);

    // 发送请求
    std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error> resp = co_await network::request::fetch_user_message_list(sdk_root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return false;
    }

    co_await handle_fetched_user_message(sdk_root, std::move(resp.value()));

    co_return true;
}

// private function impl ------------------------------------------------------------

std::unique_ptr<network::FetchUserMessageListReq> p_make_fetch_user_message_list_req(SDKRoot *root, int64_t cursor) {
    auto req = std::make_unique<network::FetchUserMessageListReq>();

    req->set_userid(root->config().user_id);
    req->set_cursor(cursor);
    req->set_limit(20);
    req->set_forward(true);
    req->set_news(true);

    return req;
}

boost::asio::awaitable<void> handle_fetched_user_message(std::weak_ptr<SDKRoot> w_root, std::unique_ptr<network::FetchUserMessageListResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_root);

    for (const auto &conv : resp->convsinfo()) {
        // 处理会话
    }

    co_return;
}

// ----------------------------------------------------------------------------------


} // namespace roc::imsdk::service