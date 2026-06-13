#ifndef IM_SDK_TEST_H
#define IM_SDK_TEST_H

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <memory>
#include "im/base/coroutine.h"
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/include/config.h"
#include "BaseConfig.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/base/include/utils/utils.h"


inline roc::imsdk::Config generateConfig();
inline roc::imsdk::model::SendMsgContext generateSendMessageContext();

std::shared_ptr<roc::imsdk::IMSDK> imsdk_t;

inline boost::asio::awaitable<void> p_test_imsdk() {

    imsdk_t = std::make_shared<roc::imsdk::IMSDK>();

    co_await imsdk_t->InitSdk(generateConfig());

    for (int i = 0; i < 3; ++i) {
        auto context = generateSendMessageContext();
        context.content = context.content + std::to_string(i);
        auto response = co_await imsdk_t->SendMessage(context, [](std::shared_ptr<roc::imsdk::model::SendMessageResponse> response) {
            std::cout << "send message response2: " << response->msg->content() << std::endl;
            std::cout << "send message response address2: " << response->msg.get() << std::endl;
            std::cout << "send message response order2: " << response->msg->server_order_index() << std::endl;
        });
    }

    auto convs = co_await imsdk_t->ConvsWhenLogin();

    auto messages = co_await imsdk_t->MessagesWhenEnterChat("0:1:12345:24680");

    // auto executor = co_await boost::asio::this_coro::executor;
    // co_await roc::base::util::switch_if_needed(net_io_context.get_executor());

    int a = 100;
    
}

inline void test_imsdk() {
    boost::asio::co_spawn(sdk_io_context, p_test_imsdk(), boost::asio::detached);
}

inline roc::imsdk::Config generateConfig() {
    roc::imsdk::Config config;
    config.app_id = "appid_0000";
    config.user_device_id = "did_0000";
    config.user_id = "12345";
    // 本地 mock 鉴权格式为 uid:<userID>；正式鉴权接入后替换为真实 token。
    config.user_token = "uid:12345";

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
