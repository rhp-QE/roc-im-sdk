#ifndef __IMSDK_CORE_MACRO_H__
#define __IMSDK_CORE_MACRO_H__

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

#endif // __IMSDK_CORE_MACRO_H__
