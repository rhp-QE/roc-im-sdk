#ifndef IM_SDK_TEST_H
#define IM_SDK_TEST_H

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include "imsdk/src/include/imsdk.h"
#include "imsdk/src/include/config.h"
#include "BaseConfig.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/include/service/message/IMessageService.h"
#include "imsdk/src/include/service/conversation/IConversationService.h"


inline roc::imsdk::Config generateConfig();
inline std::shared_ptr<roc::imsdk::model::MessageModel> generateMessage();

inline boost::asio::awaitable<void> p_test_imsdk() {
    auto imsdk = std::make_shared<roc::imsdk::IMSDK>();

    co_await imsdk->init_sdk(generateConfig());

    std::shared_ptr<roc::imsdk::model::MessageModel> message = generateMessage();
    co_await imsdk->msg_service()->send_message(message);

    while(true) {

    }
}

inline void test_imsdk() {
    boost::asio::co_spawn(net_io_context, p_test_imsdk(), boost::asio::detached);
}

inline roc::imsdk::Config generateConfig() {
    roc::imsdk::Config config;
    config.app_id = "3396";
    config.user_device_id = "67890";
    config.user_id = "12345";
    config.user_token = "token_mock";

    return config;
}

inline std::shared_ptr<roc::imsdk::model::MessageModel> generateMessage() {
    return std::make_shared<roc::imsdk::model::MessageModel>("Hello, world!", "12345", "67890", "0:12345:67890");
}

#endif