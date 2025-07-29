#ifndef ROC_IM_SDK_CORE_NETWORK_REQUEST_SDKREQUESTENUM_H
#define ROC_IM_SDK_CORE_NETWORK_REQUEST_SDKREQUESTENUM_H

#include <cstdint>

namespace roc::imsdk::network::request {


enum class SDKRequestType : int32_t {
    SEND_MESSAGE            = 101, // 发送消息
    FETCH_CONV_MESSAGE_LIST = 102, // 拉取单链
    FETCH_USER_MESSAGE_LIST = 103, // 拉取混链
};


}

#endif // ROC_IM_SDK_CORE_NETWORK_REQUEST_SDKREQUESTENUM_H