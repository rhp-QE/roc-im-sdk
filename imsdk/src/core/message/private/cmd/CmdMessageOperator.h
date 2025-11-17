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

    // 处理消息状态发生变化
    static boost::asio::awaitable<void> handle_message_status_changed(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息已读状态发生变化
    static boost::asio::awaitable<void> handle_message_read_changed(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息置顶状态发生变化
    static boost::asio::awaitable<void> handle_message_top_changed(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);
    
    // 处理消息property发生变化
    static boost::asio::awaitable<void> handle_message_property_changed(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息syncExt发生变化
    static boost::asio::awaitable<void> handle_message_sync_ext_changed(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);
    
};

}