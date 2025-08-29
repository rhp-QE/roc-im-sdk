#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::message {

class CmdMessageOperator {
public:
    static void start(W_SDK_ROOT);

private:
    static void handle_push_message(W_SDK_ROOT, std::shared_ptr<network::SdkWSResp> resp);
    
    // 处理删除消息
    static boost::asio::awaitable<void> handle_delete_message(W_SDK_ROOT, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理撤回消息
    static boost::asio::awaitable<void> handle_recall_message(W_SDK_ROOT, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理更新消息
    static boost::asio::awaitable<void> handle_update_message(W_SDK_ROOT, std::shared_ptr<network::CmdMessage> cmd_msg);

};

}