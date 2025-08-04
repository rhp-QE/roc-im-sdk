///
/// @file   UserMessageFetcher.cc
/// @brief  用户消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#include "imsdk/src/core/service/Fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/db/model/ConversationORM.h"
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
    int64_t cursor = 0;

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

    std::vector<db::MessageORM *> db_msgs;
    std::vector<db::ConversationORM *> db_convs;

    FetchUserMessageResult result;

    for (auto &conv : resp->convsinfo()) {
        // 转为orm 然后存储
        std::unique_ptr<db::ConversationORM> db_conv = convert_net_conv_to_db_conv(&conv);
        db_convs.push_back(db_conv.get());

        // 获取并更新 sdk_conv
        std::shared_ptr<model::ConversationModel> sdk_conv = sdk_root->conversation_cache()->update_and_get_sdk_conv(db_conv.get()).first;
        result.conv_messages_union_vec.push_back(injection::ConvMessagesUnion{sdk_conv, {}});

        // 获取并更新 sdk_msg
        for (auto &msg : conv.msgs()) {
            if (msg.iscmd()) {
                continue;
            }

            std::unique_ptr<db::MessageORM> db_msg = convert_net_msg_to_db_msg(&(msg.msg()));
            db_msgs.push_back(db_msg.get());

            std::shared_ptr<model::MessageModel> sdk_msg = sdk_root->message_cache()->update_and_get_sdk_message(db_msg.get()).first;
            result.conv_messages_union_vec.back().sdk_msgs.push_back(sdk_msg);
        }
        
    }


    // 插入到db
    db::operate::insert_conversation(sdk_root->database(), db_convs);
    db::operate::insert_message(sdk_root->database(), db_msgs);

    // 上抛
    co_return result;
}

// ----------------------------------------------------------------------------------


} // namespace roc::imsdk::service