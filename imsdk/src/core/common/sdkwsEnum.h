#pragma once

#include <cstdint>
namespace roc::imsdk::core::common {

static int32_t SDKWSService = 6000;
enum class SDKWSMethod : int32_t {
    SEND_MESSAGE                  = 101, /// 发送消息
    PULL_SINGLE_LIST              = 102, /// 拉取单链
    PULL_MIX_LIST                 = 103, /// 拉取混链
    PUSH_USER_MESSAGE             = 104, /// 下推用户消息
    PUSH_CMD_MESSAGE              = 105, /// 下推命令消息
    USER_MESSGAGE_INTEGRITY_CHECK = 106, /// 混链拉取会话完整性校验
    MESSAGE_CHANGE                = 107, /// 消息改变 请求
    CONVERSATION_CHANGE           = 108, /// 会话改变 请求
};

enum class MsgDStatus : int32_t {
    RealTime  = 1,
    Received  = 2,
};

/// 命令消息操作类型
enum class CmdMessageOp : int32_t {

    MSG_SEND_STATUS_CHANGED = 1001, // 消息发送状态发生变化 (发送成功、发送失败)
    MSG_READ_CHANGED        = 1002, // 已读状态发生变化
    MSG_PIN_CHANGED         = 1003, // 置顶状态发生变化
    MSG_PROPERTY_CHANGED    = 1004, // property 发生改变
    MSG_SYNC_EXT_CHANGED    = 1005, // syncExt 发生改变
    MSG_DELETE              = 1006, // 删除消息
    MSG_RECALL              = 1007, // 撤回消息

    CONV_STATUS_CHANGED     = 2001, // 会话删除状态发生变化
    CONV_READ_CHANGED       = 2002, // 会话已读状态发生变化
    CONV_TOP_CHANGED        = 2003, // 会话置顶状态发生变化
    CONV_MUTE_CHANGE        = 2004, // 会话免打扰状态发生改变
    CONV_BLOCK_CHANGE       = 2005, // 会话拉黑状态发生改变
    // CONV_PROPERTY_CHANGED   = 2004, // 会话property 发生改变
    CONV_SYNC_EXT_CHANGED   = 2006, // 会话syncExt 发生改变
};

}