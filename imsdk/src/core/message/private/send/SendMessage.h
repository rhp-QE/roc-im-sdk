#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <boost/asio/awaitable.hpp>

namespace roc::imsdk::core::message {

class SendMessage {
public:
    explicit SendMessage(std::weak_ptr<SDKRoot> sdk_root);

    /// 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> sendMessage(CONTEXT_T, model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);
 
private: 
    /// 异步发送消息
    boost::asio::awaitable<void> asyncSendMessage(CONTEXT_T, std::unique_ptr<network::SendMessageReq> req, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);

    /// 检查发送上下文
    bool checkSendContext(const model::SendMsgContext &contexts);

    /// 将发送上下文转换为网络消息
    void convertSendContextToSdkwsMessage(CONTEXT_T, model::SendMsgContext &contexts, std::string client_msg_id, double send_time, network::MsgData *req);
    
    /// 将发送上下文转换为MessageORM对象
    std::shared_ptr<MessageORM> convertSendContextToMessageOrm(CONTEXT_T, const model::SendMsgContext &context, std::string client_msg_id, double send_time);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::message