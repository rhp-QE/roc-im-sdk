#pragma once

#include <MMKV/MMKV.h>
#include "imsdk/src/core/utils/Utils.h"
#include "imsdk/src/core/macro.h"

namespace roc::imsdk {

// operator declare ------------------------------------------------------------

// 设置会话水位
inline bool set_conv_message_cursor(MMKV* mmkv, const std::string& user_id, int64_t cursor);

// 获取会话水位
inline int64_t get_conv_message_cursor(MMKV* mmkv, const std::string& user_id);

// -----------------------------------------------------------------------------



// const ----------------------------------------------------------------------
const std::string conv_message_cursor_key = "conv_message_cursor";
// ----------------------------------------------------------------------------



// operator implement -------------------------------------------------------------

inline bool set_conv_message_cursor(MMKV* mmkv, const std::string& user_id, int64_t cursor) {
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, false)

    std::string key = key_for_user(user_id, conv_message_cursor_key);
    mmkv->set(cursor, key);
}

inline int64_t get_conv_message_cursor(MMKV* mmkv, const std::string& user_id) {
    CHECK_POINTER_OR_RETURN_VALUE(mmkv, 0)

    std::string key = key_for_user(user_id, conv_message_cursor_key);
    return mmkv->getInt64(key, 0);
}

// -----------------------------------------------------------------------------

}