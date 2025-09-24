#pragma once

#include "imsdk/src/base/util.h"

// 检查 root 是否为空并直接返回的宏（无返回值）
#define CHECK_ROOT_OR_CO_RETURN_VOID(weak_ptr_var) \
    auto sdk_root = weak_ptr_var.lock(); \
    if (!sdk_root) { \
        co_return; \
    }

// 检查 root 是否为空并返回指定值的宏
#define CHECK_ROOT_OR_CO_RETURN_VALUE(weak_ptr_var, return_value) \
    auto sdk_root = weak_ptr_var.lock(); \
    if (!sdk_root) { \
        co_return return_value; \
    }

// 检查 root 是否为空并直接返回的宏（无返回值）
#define CHECK_ROOT_OR_RETURN_VOID(weak_ptr_var) \
    auto sdk_root = weak_ptr_var.lock(); \
    if (!sdk_root) { \
        return; \
    }

// 检查 root 是否为空并返回指定值的宏
#define CHECK_ROOT_OR_RETURN_VALUE(weak_ptr_var, return_value) \
    auto sdk_root = weak_ptr_var.lock(); \
    if (!sdk_root) { \
        return return_value; \
    }

// 检查指针是否为空
#define CHECK_POINTER_OR_RETURN_VOID(pointer) \
    if (!pointer) { \
        return; \
    }

#define CHECK_POINTER_OR_RETURN_VALUE(pointer, return_value) \
    if (!pointer) { \
        return return_value; \
    }

// 检查指针是否为空（协程版本）
#define CHECK_POINTER_OR_CO_RETURN_VOID(pointer) \
    if (!pointer) { \
        co_return; \
    }

#define CHECK_POINTER_OR_CO_RETURN_VALUE(pointer, return_value) \
    if (!pointer) { \
        co_return return_value; \
    }

#define W_SDK_ROOT \
    std::weak_ptr<roc::imsdk::SDKRoot> w_sdk_root 

#define CONTEXT_T \
    std::weak_ptr<roc::imsdk::SDKRoot> w_sdk_root, uint32_t call_track_id

#define CONTEXT_V \
    w_sdk_root, call_track_id

#define TRACK_ID \
    call_track_id

#define CONTEXT_NEW_V1 \
    std::weak_ptr<roc::imsdk::SDKRoot> w_sdk_root = w_sdk_root_; uint32_t call_track_id = roc::base::util::generate_uint32_random();

#define CONTEXT_NEW_V2 \
    uint32_t call_track_id = roc::base::util::generate_uint32_random();