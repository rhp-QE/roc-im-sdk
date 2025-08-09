#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"

namespace roc::imsdk::core::message {

class SendMessage {
public:
    /// 发送消息
    static boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> send_message(W_SDK_ROOT, std::vector<model::SendMsgContext> contexts);
    
private:
    /// 检查发送上下文
    static bool check_send_context(const model::SendMsgContext &contexts);
    
    /// 将发送上下文转换为网络消息
    static void convert_send_context_to_sdkws_message(W_SDK_ROOT, const model::SendMsgContext &contexts, std::string client_msg_id, network::MsgData *req);
};

} // namespace roc::imsdk::core::message