#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

namespace roc::imsdk::core::message {

class CmdMessageOperator {
public:
    explicit CmdMessageOperator(std::weak_ptr<SDKRoot> sdk_root);

    void Start(CTX_T);

private:
    void p_HandlePushMessage(CTX_T, std::shared_ptr<const network::SdkWSResp> resp);
    
    // 处理删除消息
    boost::asio::awaitable<void> p_HandleDeleteMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理撤回消息
    boost::asio::awaitable<void> p_HandleRecallMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理更新消息
    boost::asio::awaitable<void> p_HandleUpdateMessage(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息状态发生变化
    boost::asio::awaitable<void> p_HandleMessageStatusChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息已读状态发生变化
    boost::asio::awaitable<void> p_HandleMessageReadChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息置顶状态发生变化
    boost::asio::awaitable<void> p_HandleMessageTopChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);
    
    // 处理消息property发生变化
    boost::asio::awaitable<void> p_HandleMessagePropertyChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    // 处理消息syncExt发生变化
    boost::asio::awaitable<void> p_HandleMessageSyncExtChanged(CTX_T, std::shared_ptr<network::CmdMessage> cmd_msg);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

}