#ifndef IM_SDK_TEST_H
#define IM_SDK_TEST_H

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/include/config.h"
#include "BaseConfig.h"
#include "imsdk/src/include/model/message/MessageModel.h"


inline roc::imsdk::Config generateConfig();
inline roc::imsdk::model::SendMsgContext generateSendMessageContext();


inline boost::asio::awaitable<void> p_test_imsdk() {
    auto imsdk = std::make_shared<roc::imsdk::IMSDK>();

    co_await imsdk->init_sdk(generateConfig());

    for (int i = 0; i < 1; ++i) {
        auto response = co_await imsdk->send_message({generateSendMessageContext()});
    }

    while(true) {

    }
}

inline void test_imsdk() {
    boost::asio::co_spawn(net_io_context, p_test_imsdk(), boost::asio::detached);
}

inline roc::imsdk::Config generateConfig() {
    roc::imsdk::Config config;
    config.app_id = "appid_0000";
    config.user_device_id = "did_0000";
    config.user_id = "12345";
    config.user_token = "token_mock";

    return config;
}

inline roc::imsdk::model::SendMsgContext generateSendMessageContext() {
    roc::imsdk::model::SendMsgContext context;
    context.content = "Hello, world!";
    context.to_user_id = "24680";
    context.is_group_msg = false;
    return context;
}



#endif