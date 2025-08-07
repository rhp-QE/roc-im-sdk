///
/// @file   MessageSendLogic.h
/// @brief  消息发送逻辑
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#pragma once

#include <memory>

#include "base/Uncopyable.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::service {

struct SendMessageModel {
    bool is_group_message;
    std::string to_uid;
    std::string conv_id;
    std::string content;
    std::string sync_etx;
    std::string local_etx;
};

namespace message{

struct SendMessageResult {
    std::vector<std::shared_ptr<imsdk::model::MessageModel>> messages;
    std::vector<std::shared_ptr<imsdk::model::ConversationModel>> conversations;
    std::shared_ptr<roc::error::Error> error;
};

boost::asio::awaitable<SendMessageResult> send_message_v2(std::weak_ptr<SDKRoot> w_sdk_root, std::vector<std::shared_ptr<service::SendMessageModel>> send_models);

}

class MessageSendLogic : public roc::base::uncopyable {
public:
    MessageSendLogic(std::weak_ptr<SDKRoot> sdk_root);
    ~MessageSendLogic();

    asio::awaitable<std::expected<void, roc::error::Error>> 
        send_message(std::vector<std::shared_ptr<imsdk::model::MessageModel>> messages);

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk
