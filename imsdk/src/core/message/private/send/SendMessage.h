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
    /// 发送消息
    static boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> send_message(CONTEXT_T, model::SendMsgContext context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);
 
private: 
    /// 异步发送消息
    static boost::asio::awaitable<void> async_send_message(CONTEXT_T, std::unique_ptr<network::SendMessageReq> req, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);

    /// 检查发送上下文
    static bool check_send_context(const model::SendMsgContext &contexts);

    /// 将发送上下文转换为网络消息
    static void convert_send_context_to_sdkws_message(CONTEXT_T, model::SendMsgContext &contexts, std::string client_msg_id, double send_time, network::MsgData *req);
    
    /// 将发送上下文转换为MessageORM对象
    static std::shared_ptr<MessageORM> convert_send_context_to_message_orm(CONTEXT_T, const model::SendMsgContext &context, std::string client_msg_id, double send_time);
};

} // namespace roc::imsdk::core::message