#pragma once

#include <cstdint>
namespace roc::imsdk::core::common {

enum class SdkWsEnum {
    SEND_MESSAGE       = 101,
    PULL_SINGLE_LIST   = 102,
    PULL_MIX_LIST      = 103,
    PUSH_USER_MESSAGE  = 104,
    PUSH_CMD_MESSAGE   = 105,
};

enum class MsgDStatus : int32_t {
    RealTime  = 1,
    Received  = 2,
};

/// 命令消息操作类型
enum class CmdMessageOp : int32_t {
    Recall  = 1,  // 撤回
    Delete  = 2,  // 删除
    Update  = 3,  // 更新

    MSG_STATUS_CHANGED      = 1001, // 消息状态发生变化 (发送成功、发送失败、删除、撤回)
    MSG_READ_CHANGED        = 1002, // 已读状态发生变化
    MSG_TOP_CHANGED         = 1003, // 置顶状态发生变化
    MSG_PROPERTY_CHANGED    = 1004, // property 发生改变
    MSG_SYNC_EXT_CHANGED    = 1005, // syncExt 发生改变

    CONV_STATUS_CHANGED     = 2001, // 会话删除状态发生变化
    CONV_READ_CHANGED       = 2002, // 会话已读状态发生变化
    CONV_TOP_CHANGED        = 2003, // 会话置顶状态发生变化
    // CONV_PROPERTY_CHANGED   = 2004, // 会话property 发生改变
    CONV_SYNC_EXT_CHANGED   = 2005, // 会话syncExt 发生改变
};

enum class CmdConvOp : int32_t {
    SetTop        = 1,  // 置顶
    CancelTop     = 2,  // 取消置顶
    SetMute       = 3,  // 设置静音
    CancelMute    = 4,  // 取消静音
    SetDelete     = 5,  // 设置删除
    CancelDelete  = 6,  // 取消删除
    SetBlock      = 7,  // 设置屏蔽
    CancelBlock   = 8,  // 取消屏蔽
};

}

namespace roc::imsdk::network {
    
enum class SDKRequestType : int32_t {
    SEND_MESSAGE            = 101, // 发送消息
    FETCH_CONV_MESSAGE_LIST = 102, // 拉取单链
    FETCH_USER_MESSAGE_LIST = 103, // 拉取混链
};
    
};