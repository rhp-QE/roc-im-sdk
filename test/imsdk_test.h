#ifndef IM_SDK_TEST_H
#define IM_SDK_TEST_H

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include "im/base/coroutine.h"
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/include/config.h"
#include "BaseConfig.h"
#include "imsdk/src/include/model/message/MessageModel.h"


inline roc::imsdk::Config generateConfig();
inline roc::imsdk::model::SendMsgContext generateSendMessageContext();


inline boost::asio::awaitable<void> p_test_imsdk() {
    auto imsdk = std::make_shared<roc::imsdk::IMSDK>();

    co_await imsdk->init_sdk(generateConfig());

    for (int i = 0; i < 3; ++i) {
        auto context = generateSendMessageContext();
        context.content = context.content + std::to_string(i);
        auto response = co_await imsdk->send_message(context, [](std::shared_ptr<roc::imsdk::model::SendMessageResponse> response) {
            std::cout << "send message response2: " << response->msg->content() << std::endl;
            std::cout << "send message response address2: " << response->msg.get() << std::endl;
            std::cout << "send message response order2: " << response->msg->server_order_index() << std::endl;
        });
    }

    // auto convs = co_await imsdk->convs_when_login();

    // auto messages = co_await imsdk->messages_when_enter_chat("0:1:12345:24680");

    int a = 100;
    while(true) {

    }
}

inline void test_imsdk() {
    boost::asio::co_spawn(main_io_context, p_test_imsdk(), boost::asio::detached);
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