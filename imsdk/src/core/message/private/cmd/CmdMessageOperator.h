#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::message {

class CmdMessageOperator {
public:
    explicit CmdMessageOperator(std::weak_ptr<SDKRoot> sdk_root);

    void Start(CONTEXT_T);

private:
    void p_HandlePushMessage(CONTEXT_T, std::shared_ptr<network::SdkWSResp> resp);
    
    // 处理删除消息
    boost::asio::awaitable<void> p_HandleDeleteMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理撤回消息
    boost::asio::awaitable<void> p_HandleRecallMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理更新消息
    boost::asio::awaitable<void> p_HandleUpdateMessage(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息状态发生变化
    boost::asio::awaitable<void> p_HandleMessageStatusChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息已读状态发生变化
    boost::asio::awaitable<void> p_HandleMessageReadChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息置顶状态发生变化
    boost::asio::awaitable<void> p_HandleMessageTopChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);
    
    // 处理消息property发生变化
    boost::asio::awaitable<void> p_HandleMessagePropertyChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息syncExt发生变化
    boost::asio::awaitable<void> p_HandleMessageSyncExtChanged(CONTEXT_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

}