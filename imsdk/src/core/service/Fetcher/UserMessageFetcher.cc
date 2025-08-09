///
/// @file   UserMessageFetcher.cc
/// @brief  用户消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/db/model/ConversationORM.h"
#include "imsdk/src/core/service/Range/ConversationRange.h"
#include "imsdk/src/core/db/model/MessageORM.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/utils/Utils.h"
#include "imsdk/src/core/db/operator/MessageOperator.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::service {

// private function declare ----------------------------------------------------------
std::unique_ptr<network::FetchUserMessageListReq> p_make_fetch_user_message_list_req(SDKRoot *root, int64_t cursor);
boost::asio::awaitable<FetchUserMessageResult> handle_fetched_user_message(std::weak_ptr<SDKRoot> w_root, std::unique_ptr<network::FetchUserMessageListResp> resp);
// ----------------------------------------------------------------------------------

UserMessageFetcher::UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root) : sdk_root_(sdk_root) {
}

UserMessageFetcher::~UserMessageFetcher() = default;

asio::awaitable<FetchUserMessageResult> UserMessageFetcher::fetch_user_messages() {
    CHECK_ROOT_OR_CO_RETURN_VALUE(sdk_root_, false);

    // 获取最新游标
    int64_t cursor = sdk_root->conversation_range()->conv_cursor();

    // 构造请求
    std::unique_ptr<network::FetchUserMessageListReq> req = p_make_fetch_user_message_list_req(sdk_root.get(), cursor);

    // 发送请求
    std::expected<std::unique_ptr<network::FetchUserMessageListResp>, roc::error::Error> resp = co_await network::request::fetch_user_message_list(sdk_root.get(), req.get());
    if (!resp || !resp.has_value()) {
        co_return false;
    }

    FetchUserMessageResult result = co_await handle_fetched_user_message(sdk_root, std::move(resp.value()));

    co_return result;
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

boost::asio::awaitable<FetchUserMessageResult> handle_fetched_user_message(std::weak_ptr<SDKRoot> w_root, std::unique_ptr<network::FetchUserMessageListResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_root, FetchUserMessageResult());

    std::vector<std::shared_ptr<db::MessageORM>> db_msgs;
    std::vector<std::shared_ptr<db::ConversationORM>> db_convs;



    for (auto &conv : resp->convsinfo()) {
        // 转为orm 然后存储
        std::shared_ptr<db::ConversationORM> db_conv = util::convert_net_conv_to_db_conv(&conv);
        db_convs.push_back(db_conv);

        // 获取并更新 sdk_msg
        for (auto &msg : conv.msgs()) {
            if (msg.iscmd()) {
                continue;
            }

            std::shared_ptr<db::MessageORM> db_msg = util::convert_net_msg_to_db_msg(&(msg.msg()));
            db_msgs.push_back(db_msg);
        }
    }

    auto sdk_msgs = sdk_root->message_cache()->update_and_get_sdk_message(db_msgs);
    auto sdk_convs = sdk_root->conversation_cache()->update_and_get_sdk_conv(db_convs);

    std::vector<injection::ConvMessagesUnion> conv_msgs_union_vec = util::convert_sdk_msg_to_conv_msgs_union(w_root, sdk_msgs);

    // 更新会话区间
    sdk_root->conversation_range()->update_conv_cursor(resp->cursor());

    FetchUserMessageResult result {
        .has_more = resp->hasmore(),
        .conv_msgs_union_vec = conv_msgs_union_vec
    };

    // 上抛
    co_return result;
}

// ----------------------------------------------------------------------------------


} // namespace roc::imsdk::service