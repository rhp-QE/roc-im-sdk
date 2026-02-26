#include "imsdk/src/core/conversation/private/fetcher/UserMessageFetcher.h"

#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/conversation/private/db_opt/DBOpt.h"
#include "imsdk/src/core/conversation/private/datasource/ConvDatasource.h"
#include "imsdk/src/core/conversation/private/receive/ReceiveConversation.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/include/model/network.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <cstdint>
#include <memory>
#include <atomic>
#include <expected>

namespace roc::imsdk::core::conversation {

struct FetchUserMessageResult {};

UserMessageFetcher::UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root)
    : w_sdk_root(sdk_root) {}

asio::awaitable<void> UserMessageFetcher::FetchUserMessages(CTX_T) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    if (is_pulling) {
        co_return;
    }

    int cnt = 10;
    bool is_news = true;
    bool has_more = true;
    bool forward = true;
    int64_t cursor = sdk_root->ConversationManager()->db_opt->ChatsCursor(CTX_V); // TODO 使用本地水位
    int64_t left = -1, right = -1; // 本次混链拉取的会话区间

    /// 第一次请求 is_news = true, 拉取区间为 【cursor, *】 的消息， 由于区间可能过大， 服务端触发分页拉取 返回 区间为【left，right】 的消息 （left > cursor）
    /// 如果触发分页拉取，is_news = false，则拉取区间为 【cursor, left】的消息 
    /// has_more = false, 则表示拉取完成。最终区间【left(最后一次拉取返回值), right(第一次拉取返回值)】 left = cursor。
    /// 消息检验： 【left, right】区间的会话 发送给服务端， 服务的那进行校验 补充空洞。 空洞补齐后 更新会话cursor 为 right
    /// note: left， right 为左闭右开区间

    do {
        // 构造请求
        std::unique_ptr<network::FetchUserRecentConvListRequest> req = p_makeFetchUserMessageListReq(CTX_V, is_news, cursor, forward);

        // 发送请求
        std::expected<std::unique_ptr<network::FetchUserRecentConvListResponse>, roc::error::Error> resp = co_await p_request(CTX_V, req.get());
        if (!resp.has_value()) {
            co_return;
        }

        if (is_news) {
            right = resp.value()->right();
            is_news = false;
        }
        if (!resp.value()->error().empty()) {
            left = resp.value()->left();
        }

        cursor = resp.value()->left();
        has_more = resp.value()->conversations_size() > 0 && resp.value()->error().empty();

        const int size = resp.value()->conversations_size();
        std::vector<std::shared_ptr<network::ConversationData>> net_convs;
        net_convs.reserve(size);
        for (int i = size - 1; i >= 0; --i) {
            network::ConversationData* raw = resp.value()->mutable_conversations()->ReleaseLast();
            std::shared_ptr<network::ConversationData> conv(raw);
            net_convs.push_back(conv);
            pulled_convs_.push_back(conv->convid());
        }

        // 处理请求
        asio::co_spawn(sdk_root->sdk_io_context(), p_handleFetchedUserMessage(CTX_V, net_convs), asio::detached);
    } while (has_more && (cnt--) > 0);

    // 会话区间二次校验【left, right】
    bool is_integrity = co_await p_doubleCheckUserMessageIntegrity(CTX_V, left, right);

    // 存储本地水位 right
    if (is_integrity) {
        sdk_root->ConversationManager()->db_opt->SetChatsCursor(CTX_V, right);
    }

    LOG_INFO("UserMessageFetcher", "user message fetch integrity = {}", is_integrity);

    co_return;
}

// =================================== private ===========================================================

boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchUserRecentConvListResponse>, roc::error::Error>> 
UserMessageFetcher::p_request(CTX_T, network::FetchUserRecentConvListRequest *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    // 创建 FrontierMessage 请求
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::PULL_MIX_LIST));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = request->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    request->SerializeToArray(frontier_msg->payload.data(), payload_size);

    std::expected<std::unique_ptr<network::FrontierMessage>, roc::error::Error> response =
        co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }

    // 从响应的 payload 中解析 FetchUserRecentConvListResponse
    auto resp = std::make_unique<network::FetchUserRecentConvListResponse>();
    // 直接使用 vector 中的数据解析，避免拷贝
    bool ok = resp->ParseFromArray(response.value()->payload.data(), response.value()->payload.size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40203, "SDKRequest fetch_user_message_list parse response failed"));
    }

    co_return resp;
}

std::unique_ptr<network::FetchUserRecentConvListRequest> UserMessageFetcher::p_makeFetchUserMessageListReq(CTX_T, bool news, int64_t cursor, bool forward) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchUserRecentConvListRequest>();

    req->set_userid(sdk_root->config().user_id);
    req->set_lowerversion(cursor);
    req->set_upperversion(cursor + 100);
    req->set_first(news);

    return req;
}

boost::asio::awaitable<void> UserMessageFetcher::p_handleFetchedUserMessage(CTX_T, std::vector<std::shared_ptr<network::ConversationData>> net_convs) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    std::vector<std::shared_ptr<network::MessageData>> net_msgs;

    for (auto &conv : net_convs) {
        while (conv->messages_size() > 0) {
            auto* msg = conv->mutable_messages()->ReleaseLast();
            net_msgs.push_back(std::shared_ptr<network::MessageData>(msg));
        }
    }

    LOG_INFO("ConvManager", "finish_fetch_user_message, net_msgs: {}, net_convs: {}", net_msgs.size(), net_convs.size());

    // 处理接收到的消息
    sdk_root->MessageManager()->HandleReceiveMessage(CTX_V, net_msgs);

    // 处理接收到的会话
    auto conv_manager = sdk_root->ConversationManager();
    co_spawn(sdk_root->sdk_io_context(), conv_manager->receive_conversation->HandleReceiveConversation(CTX_V, std::move(net_convs)), asio::detached);

    // 上抛
    co_return;
}


boost::asio::awaitable<bool> UserMessageFetcher::p_doubleCheckUserMessageIntegrity(CTX_T, int64_t left, int64_t right) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, false)

    std::unique_ptr<network::UserMessageIntegrityCheckRequest> req_data = std::make_unique<network::UserMessageIntegrityCheckRequest>();
    req_data->set_userid(sdk_root->config().user_id);
    req_data->set_left(left);
    req_data->set_right(right);
    for (const std::string& convID : pulled_convs_) {
        req_data->add_convids(convID);
    }

    // 创建 FrontierMessage 请求
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::USER_MESSGAGE_INTEGRITY_CHECK));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = req_data->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    req_data->SerializeToArray(frontier_msg->payload.data(), payload_size);

    // 发送请求（type、timestamp、track_id 会在 ConnectionManager 内设置）
    std::expected<std::unique_ptr<network::FrontierMessage>, roc::error::Error> resp =
        co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!resp.has_value()) {
        co_return false;
    }

    // 从响应的 payload 中解析 UserMessageIntegrityCheckResp
    std::unique_ptr<network::UserMessageIntegrityCheckResponse> resp_data = std::make_unique<network::UserMessageIntegrityCheckResponse>();
    // 直接使用 vector 中的数据解析，避免拷贝
    if (!resp_data->ParseFromArray(resp.value()->payload.data(), resp.value()->payload.size())) {
        co_return false;
    }

    if (resp_data->isintegrity()) {
        co_return true;
    }

    std::string log_str;
    const int size = resp_data->conversations_size();
    std::vector<std::shared_ptr<network::ConversationData>> net_convs;
    net_convs.reserve(size);
    for (int i = size - 1; i >= 0; --i) {
        network::ConversationData* raw = resp_data->mutable_conversations()->ReleaseLast();
        std::shared_ptr<network::ConversationData> conv(raw);
        net_convs.push_back(conv);
        log_str += conv->convid() + " | ";
    }
    LOG_INFO("UserMessageFetcher", "trigger user message complete, convIDS = {}", log_str);

    // 处理会话
    boost::asio::co_spawn(sdk_root->sdk_io_context(), p_handleFetchedUserMessage(CTX_V, std::move(net_convs)), asio::detached);

    co_return true;
}

} // namespace roc::imsdk::core::conversation