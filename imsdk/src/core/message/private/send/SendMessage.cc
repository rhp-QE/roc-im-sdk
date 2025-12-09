#include "imsdk/src/core/message/private/send/SendMessage.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/util.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/core/message/private/convert/Convert.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/message/private/data_source/MessageDataSource.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/include/model/network.h"
#include <algorithm>
#include <expected>
#include <memory>
#include <utility>

namespace roc::imsdk::core::message {

SendMessageController::SendMessageController(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

boost::asio::awaitable<std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error>> 
SendMessageController::p_request(CTX_T, network::SendMessageReq *request) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error("sdk root is empty")))

    // 创建 FrionterMessage 请求
    auto frontier_msg = std::make_unique<network::FrionterMessage>();
    frontier_msg->service = core::common::SDKWSService;
    frontier_msg->method  = std::to_string(static_cast<int32_t>(common::SDKWSMethod::SEND_MESSAGE));
    // 使用 SerializeToArray 避免数据拷贝，直接写入 vector
    int payload_size = request->ByteSizeLong();
    frontier_msg->payload.resize(payload_size);
    request->SerializeToArray(frontier_msg->payload.data(), payload_size);
    
    // 将 track_id 放在 metadata 中
    frontier_msg->metadata["track_id"] = std::to_string(TRACK_ID);

    // 发送请求（type 和 timestamp 会在 ConnectionManager 内设置）
    std::expected<std::unique_ptr<network::FrionterMessage>, roc::error::Error> response = co_await sdk_root->ConnectionManager()->SendRequest(std::move(frontier_msg));
    if (!response.has_value()) {
        co_return std::unexpected(response.error());
    }

    // 从响应的 payload 中解析 SendMessageResp
    auto resp = std::make_unique<network::SendMessageResp>();
    // 直接使用 vector 中的数据解析，避免拷贝
    bool ok = resp->ParseFromArray(response.value()->payload.data(), response.value()->payload.size());
    if (!ok) {
        co_return std::unexpected(roc::error::make_error(40201, "SDKRequest send_message parse response failed"));
    }

    co_return resp;
}

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> SendMessageController::SendMessage(CTX_T, model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    std::shared_ptr<model::SendMessageResponse> response = std::make_shared<model::SendMessageResponse>(1, "", nullptr);

    /// 检查数据是否合法
    if (!p_checkSendContext(context)) {
        response->error_msg = "invalid context";
        co_return response;
    } 

    auto msg_manager = sdk_root->MessageManager();
    double send_time = util::current_time_since1970();
    std::string client_msg_id = p_generateClientMsgId(); 


    std::unique_ptr<network::SendMessageReq> req = std::make_unique<network::SendMessageReq>();
    p_convertSendContextToSdkwsMessage(CTX_V, context, client_msg_id, send_time, req->add_msgs());

    LOG_INFO("MsgManager", "call_asyncSendMessage, is_group_msg: {}, conv_id: {}, from: {}, to: {}", context.is_group_msg, context.conv_id, context.from_user_id, context.to_user_id);

    {
        auto db_msg = p_convertSendContextToMessageOrm(CTX_V, context, client_msg_id, send_time);
        auto sdk_msgs = co_await msg_manager->message_data_source->SaveDbMsgs(CTX_V, {db_msg});
        if (sdk_msgs.empty()) {
            response->error_msg = "save message failed";
            co_return response;
        }

        response->error_code = 0;
        response->msg = sdk_msgs.front();
    } // 保存db 然后先返回给用户

    { 
        boost::asio::co_spawn(sdk_root->net_io_context(), p_asyncSendMessage(CTX_V, std::move(req), std::move(callback)), boost::asio::detached);
    } // 构造请求 异步发送数据


    co_return response;
}

boost::asio::awaitable<void> SendMessageController::p_asyncSendMessage(CTX_T, std::unique_ptr<network::SendMessageReq> req, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root);

    std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error> resp = co_await p_request(CTX_V, req.get());

    if (!resp || !resp.has_value() || resp.value()->infos().size() != 1) {
        base::util::safe_invoke_block(callback, std::make_shared<model::SendMessageResponse>(false, resp.error().message(), nullptr));
        co_return;
    }

    auto msg_manager = sdk_root->MessageManager();
    auto sdk_msgs = co_await msg_manager->message_data_source->SaveNetMessages(CTX_V, {&(resp.value()->infos()[0].msg())});

    LOG_INFO("MsgManager", "asyncSendMessage, result: {}", sdk_msgs.empty() ? "failed" : "success");

    if (sdk_msgs.empty()) {
        base::util::safe_invoke_block(callback, std::make_shared<model::SendMessageResponse>(false, "save message failed", nullptr));
        co_return;
    }

    base::util::safe_invoke_block(callback, std::make_shared<model::SendMessageResponse>(true, "", sdk_msgs.front()));
    co_return;
}

// ----------------------------- private static methods -----------------------------

std::string SendMessageController::p_generateClientMsgId() {
    return base::util::uuid();
}

bool SendMessageController::p_checkSendContext(const model::SendMsgContext &context) {
    if (context.content.empty()) {
        return false;
    }
    
    if (context.is_group_msg && context.conv_id.empty()) {
        return false;
    } 
    
    if (!context.is_group_msg && context.conv_id.empty() && context.to_user_id.empty()) {
        return false;
    }
    
    return true;
}

void SendMessageController::p_convertSendContextToSdkwsMessage(CTX_T, model::SendMsgContext &context, std::string client_msg_id, double send_time, network::MsgData *net_msg) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    if (!net_msg) {
        return;
    }

    net_msg->set_clientmsgid(client_msg_id);
    net_msg->set_content(context.content);
    net_msg->set_senderplatformid(1);
    net_msg->set_msgfrom(100);
    net_msg->set_contenttype(101);
    net_msg->set_content(context.content);
    net_msg->set_sendid(sdk_root->config().user_id);
    net_msg->set_sendtime(send_time);

    if (context.is_group_msg) {
        net_msg->set_convid(context.conv_id);
    } else {
        std::string conv_id = context.conv_id.empty() ? core::util::generate_single_conv_id(sdk_root->config().user_id, context.to_user_id) : context.conv_id;

        std::pair<std::string, std::string> user_ids = core::util::parse_single_conv_id(conv_id);
        std::string receiver_id = user_ids.first;
        if (user_ids.first == sdk_root->config().user_id) {
            receiver_id = user_ids.second;
        }

        net_msg->set_recvid(receiver_id);
        net_msg->set_convid(conv_id);

        context.conv_id = conv_id;
        context.to_user_id = receiver_id;
        context.from_user_id = sdk_root->config().user_id;
    }
}   

std::shared_ptr<MessageORM> SendMessageController::p_convertSendContextToMessageOrm(CTX_T, const model::SendMsgContext &context, std::string client_msg_id, double send_time) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, nullptr);
    
    auto message_orm = std::make_shared<MessageORM>();
    
    // 设置基本信息
    message_orm->content = context.content;
    message_orm->client_msg_id = client_msg_id;
    message_orm->server_order_index = 0;  /// 设置为0 保证本地 ranges不变， 等response 回来后再更新
    message_orm->conversation_id = context.conv_id;
    message_orm->from_user_id = sdk_root->config().user_id; // 从SDKRoot获取当前用户ID
    message_orm->to_user_id = context.to_user_id;
    
    // 设置消息类型
    message_orm->is_group_msg = context.is_group_msg;
    
    // 设置扩展信息
    message_orm->sync_ext = context.sync_ext;
    message_orm->local_ext = context.local_ext;
    
    // 设置状态和标志
    message_orm->status = 0; // 默认状态
    message_orm->is_pinned = false;
    message_orm->is_deleted = false;
    message_orm->is_recalled = false;
    
    // 设置时间戳
    message_orm->send_time = send_time;
    
    // 设置顺序索引
    message_orm->client_order_index = msg_manager->db_opt->NextMsgOrderInConv(CTX_V, context.conv_id);
    
    return message_orm;
}

// -------------------------------------------------------------------------------------

} // namespace roc::imsdk::core::message
