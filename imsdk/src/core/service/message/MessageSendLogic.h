///
/// @file   MessageSendLogic.h
/// @brief  消息发送逻辑
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#ifndef __IMSDK_MESSAGE_SERVICE_CORE_MESSAGE_SEND_LOGIC_H__
#define __IMSDK_MESSAGE_SERVICE_CORE_MESSAGE_SEND_LOGIC_H__

#include <memory>

#include "base/Uncopyable.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::service {

class MessageSendLogic : public roc::base::uncopyable {
public:
    MessageSendLogic(std::weak_ptr<SDKRoot> sdk_root);
    ~MessageSendLogic();

    asio::awaitable<std::expected<void, roc::error::Error>> send_message(std::vector<std::shared_ptr<imsdk::model::MessageModel>> messages);

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk

#endif