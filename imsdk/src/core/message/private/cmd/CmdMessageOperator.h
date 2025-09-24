#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::message {

class CmdMessageOperator {
public:
    static void start(CONTEXT_T);

private:
    static void handle_push_message(CONTEXT_T, std::shared_ptr<network::SdkWSResp> resp);
    
    // 处理删除消息
    static boost::asio::awaitable<void> handle_delete_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理撤回消息
    static boost::asio::awaitable<void> handle_recall_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理更新消息
    static boost::asio::awaitable<void> handle_update_message(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

};

}