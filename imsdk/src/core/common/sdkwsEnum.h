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

enum class CmdMessageOp : int32_t {
    Recall  = 1,  // 撤回
    Delete  = 2,  // 删除
    Update  = 3,  // 更新
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