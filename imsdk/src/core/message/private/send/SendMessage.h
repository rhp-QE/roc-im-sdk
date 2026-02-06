#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/base/include/network/Error.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>

namespace roc::imsdk::core::message {

class SendMessageController {
public:
    explicit SendMessageController(std::weak_ptr<SDKRoot> sdk_root);

    /// 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>>
        SendMessage(CTX_T, model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);
 
private: 
    /// 生成客户端消息 ID
    std::string p_generateClientMsgId();

    /// 发送消息请求
    boost::asio::awaitable<std::expected<std::unique_ptr<network::BatchSendMessageResponse>, roc::error::Error>> 
        p_request(CTX_T, std::unique_ptr<network::BatchSendMessageRequest> request);
    
    /// 异步发送消息
    boost::asio::awaitable<void> p_asyncSendMessage(CTX_T, std::unique_ptr<network::BatchSendMessageRequest> req, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);

    /// 检查发送上下文
    bool p_checkSendContext(const model::SendMsgContext &contexts);

    /// 将发送上下文转换为网络消息
    void p_convertSendContextToSdkwsMessage(CTX_T, model::SendMsgContext &contexts, std::string client_msg_id, double send_time, network::MessageData *req);
    
    /// 将发送上下文转换为MessageORM对象
    std::shared_ptr<MessageORM> p_convertSendContextToMessageOrm(CTX_T, const model::SendMsgContext &context, std::string client_msg_id, double send_time);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::message