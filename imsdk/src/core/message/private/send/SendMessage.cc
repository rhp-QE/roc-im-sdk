#include "imsdk/src/core/message/private/send/SendMessage.h"

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/network/request/SDKRequest.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/message/private/save/SaveMessage.h"

#include <memory>

namespace roc::imsdk::core::message {

boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> SendMessage::send_message(W_SDK_ROOT, std::vector<model::SendMsgContext> contexts) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    std::unordered_map<std::string, std::shared_ptr<model::SendMessageResponse>> send_response_map;
    std::unique_ptr<network::SendMessageReq> req = std::make_unique<network::SendMessageReq>();

    for (auto &context : contexts) {

        std::string client_msg_id = message::SaveMessage::generate_client_msg_id();
        send_response_map[client_msg_id] = std::make_shared<model::SendMessageResponse>(false, "", nullptr);

        /// 检查 context 是否合法
        if (!check_send_context(context)) {
            send_response_map[client_msg_id]->error_msg = "invalid context";
            continue;
        }

        /// 将 context 转为 net_msg
        convert_send_context_to_sdkws_message(w_sdk_root, context, client_msg_id, req->add_msgs());
    }

    /// 发送消息
    std::expected<std::unique_ptr<network::SendMessageResp>, roc::error::Error> resp = co_await network::request::send_message(sdk_root.get(), req.get());

    std::vector<const network::MsgData *> net_msgs;
    if (!resp || !resp.has_value()) {
        for (auto &[client_msg_id, response] : send_response_map) {
            response->is_success = false;
            response->error_msg = resp.error().message();
            response->msg = nullptr;
        }
        co_return send_response_map;
    }

    for (const auto &info : resp.value()->infos()) {
        if (info.errorcode() == "0") {
            net_msgs.push_back(&info.msg());
        } 
    }

    /// 保存消息
    auto sdk_msgs = message::SaveMessage::save_net_msgs(w_sdk_root, net_msgs);

    /// 更新发送结果
    for (auto &sdk_msg : sdk_msgs) {
        std::string client_msg_id = sdk_msg->client_msg_id();
        if (send_response_map.find(client_msg_id) != send_response_map.end()) {
            continue;
        }

        send_response_map[client_msg_id]->is_success = true;
        send_response_map[client_msg_id]->msg = sdk_msg;
    }

    co_return send_response_map;
}

// ----------------------------- private static methods -----------------------------

bool SendMessage::check_send_context(const model::SendMsgContext &context) {
    if (context.content.empty()) {
        return false;
    }
    if (context.conv_id.empty()) {
        return false;
    }
    if (context.to_user_id.empty()) {
        return false;
    }
    return true;
}

void SendMessage::convert_send_context_to_sdkws_message(W_SDK_ROOT, const model::SendMsgContext &context, std::string client_msg_id, network::MsgData *net_msg) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    if (!net_msg) {
        return;
    }

    net_msg->set_clientmsgid(client_msg_id);
    net_msg->set_content(context.content);
    net_msg->set_senderplatformid(1);
    net_msg->set_msgfrom(100);
    net_msg->set_contenttype(101);
    net_msg->set_sendtime(time(nullptr));
    net_msg->set_sendid(sdk_root->config().user_id);
    net_msg->set_recvid(context.to_user_id);
    net_msg->set_convid(context.conv_id);
    net_msg->set_content(context.content);
}   

// -------------------------------------------------------------------------------------

} // namespace roc::imsdk::core::message