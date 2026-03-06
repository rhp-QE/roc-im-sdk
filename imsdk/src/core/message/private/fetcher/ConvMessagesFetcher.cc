#include "ConvMessagesFetcher.h"

#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/src/core/message/private/receive/ReceiveMessage.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/include/model/network.h"

#include <algorithm>
#include <atomic>
#include <expected>

namespace roc::imsdk::core::message {

ConvMessagesFetcher::ConvMessagesFetcher(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

// =================================== private ===========================================================

boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResponse>, roc::error::Error>> 
ConvMessagesFetcher::p_request(CTX_T, network::FetchConvMessageListRequest *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    // 创建 FrontierMessage 请求
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::PULL_SINGLE_LIST));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = request->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    request->SerializeToArray(frontier_msg->payload.data(), payload_size);

    std::expected<std::unique_ptr<network::FrontierMessage>, roc::error::Error> response =
        co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }

    // 从响应的 payload 中解析 FetchConvMessageListResponse
    auto resp = std::make_unique<network::FetchConvMessageListResponse>();
    // 直接使用 vector 中的数据解析，避免拷贝
    bool ok = resp->ParseFromArray(response.value()->payload.data(), response.value()->payload.size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40202, "SDKRequest fetch_conv_message_list parse response failed"));
    }

    co_return resp;
}

// 生成请求
std::unique_ptr<network::FetchConvMessageListRequest> ConvMessagesFetcher::p_MakeFetchConvMessageListReq(CTX_T, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto req = std::make_unique<network::FetchConvMessageListRequest>();

    req->set_userid(sdk_root->config().user_id);
    req->set_convid(conv_id);
    req->set_mode(1);
    req->set_left(range.first);
    req->set_right(range.second);

    return req;
}

// 处理返回数据
void ConvMessagesFetcher::p_HandleFetchConvMessgaeListResp(CTX_T, std::unique_ptr<network::FetchConvMessageListResponse> resp) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    std::vector<std::shared_ptr<network::MessageData>> net_msgs;

    while (resp->messages_size() > 0) {
        auto* msg = resp->mutable_messages()->ReleaseLast();
        net_msgs.push_back(std::shared_ptr<network::MessageData>(msg));
    }

    LOG_INFO("MsgManager", "call_track_id: {}, handle_fetchConvMessageList_resp, size: {}", TRACK_ID, net_msgs.size());

    // 保存消息
    auto msg_manager = sdk_root->MessageManager();
    boost::asio::co_spawn(sdk_root->net_io_context(), msg_manager->receive_message->HandleReceiveMessage(CTX_V, net_msgs), boost::asio::detached);
}

asio::awaitable<void> ConvMessagesFetcher::FetchConvMessageListForRange(CTX_T, std::string conv_id, std::pair<int64_t, int64_t> range) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);
    
    // 防止死循环
    int cnt = 10;
    bool has_more = true;
    do {
        std::unique_ptr<network::FetchConvMessageListRequest> req = p_MakeFetchConvMessageListReq(CTX_V, conv_id, range);
        if (!req) {
            break;
        }

        // 发送请求
        std::expected<std::unique_ptr<network::FetchConvMessageListResponse>, roc::error::Error> resp = 
            co_await p_request(CTX_V, req.get());
        if (!resp || !resp.has_value()) {
            continue;
        }
        has_more = resp.value()->havemore();
        
        // 处理数据
        p_HandleFetchConvMessgaeListResp(CTX_V, std::move(resp.value()));

    } while(cnt-- > 0 && has_more);
}

// ==========================================================================================================

boost::asio::awaitable<void> ConvMessagesFetcher::FetchConvMessageList(CTX_T, std::string conv_id) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    auto msg_manager = sdk_root->MessageManager();
    auto msg_empty_ranges = msg_manager->message_data_source->EmptyMessageRangeForConvId(CTX_V, conv_id);

    for (const auto &range : msg_empty_ranges) {

        LOG_INFO("MsgManager", "start_fetchConvMessageList, conv_id: {}, range: {{{}, {}}}", conv_id, range.first, range.second);

        co_await FetchConvMessageListForRange(CTX_V, conv_id, range);
    }

    std::cout<<"fetch conv message success"<<std::endl;
}

// ==========================================================================================================

} // namespace roc::imsdk::core::message