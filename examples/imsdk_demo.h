#pragma once

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
#include "base/utils/utils.h"

int64_t message_cursor = -1;
int64_t conv_cursor = -1;

inline roc::imsdk::Config generateConfig(std::string user_id);
inline roc::imsdk::model::SendMsgContext generateSendMessageContext(std::string to_user_id, std::string content);

std::shared_ptr<roc::imsdk::IMSDK> imsdk;

inline void print_msg(std::shared_ptr<const roc::imsdk::model::MessageModel> msg) {
    std::cout << "      ----------------------------------------" << std::endl;
    std::cout << "      [message_id:   ] " << msg->client_msg_id() << std::endl;
    std::cout << "      [message_order:] " << msg->client_order_index() << std::endl;
    std::cout << "      [content:      ] " << msg->content() << std::endl;
    std::cout << "      [from:         ] " << msg->from_user_id() << std::endl;
    std::cout << "      [to:           ] " << msg->to_user_id() << std::endl;
    std::cout << "      ----------------------------------------" << std::endl;
}


inline boost::asio::awaitable<bool> p_login() {
    std::cout << "请输入用户ID: ";
    std::string user_id;
    std::cin >> user_id;

    auto config = generateConfig(user_id);
    imsdk = std::make_shared<roc::imsdk::IMSDK>();
    co_await imsdk->init_sdk(config);

    imsdk->on_messagee([](roc::imsdk::model::OnMessageResult result) {
        std::cout << "\n ==============消息更新 (begin) =============" << std::endl;
        std::cout << " 实时消息: " << result.real_time_msgs.size() << std::endl;
        for (auto msg : result.real_time_msgs) {
            print_msg(msg);
        }
        std::cout << " 离线消息 (未被接收过): " << result.offline_not_received_msgs.size() << std::endl;
        for (auto msg : result.offline_not_received_msgs) {
            print_msg(msg);
        }
        std::cout << " 离线消息 (已接收过): " << result.offline_received_msgs.size() << std::endl;
        for (auto msg : result.offline_received_msgs) {
            print_msg(msg);
        }
        std::cout << " ==============消息更新 (end) =============\n" << std::endl;
    });

    co_return true;
}

inline boost::asio::awaitable<void> chat_first_page() {
    std::cout << "登录成功 正在拉取会话列表...." << std::endl;
    auto convs = co_await imsdk->convs_when_login();
    conv_cursor = convs->cursor;

    std::cout<<"\n========首屏会话 (begin) ========"<<std::endl;
    // 打印会话信息
    for(auto conv : convs->convs) {
        std::cout << "      [conv_id: ] " << conv->conversation_id() << std::endl;
    }
    std::cout<<"========首屏会话 (end) ========\n"<<std::endl;
}

inline boost::asio::awaitable<void> p_login_out() {
    co_await imsdk->login_out();
}

inline boost::asio::awaitable<void> p_enter_chat() {
    std::cout << "请输入会话ID: ";
    std::string conv_id;
    std::cin >> conv_id;

    auto messages = co_await imsdk->messages_when_enter_chat(conv_id);
    message_cursor = messages->cursor;

    std::cout<<"\n========进入会话 (begin) ========"<<std::endl;
    for(auto message : messages->messages) {
        print_msg(message);
    }
    std::cout<<"========进入会话 (end) ========\n"<<std::endl;
}

inline boost::asio::awaitable<void> p_send_message() {
    std::cout << "请输入接收者ID: ";
    std::string to_user_id;
    std::cin >> to_user_id;
    std::cout << "请输入消息内容: ";
    std::string content;
    std::cin >> content;
    auto context = generateSendMessageContext(to_user_id, content);
    auto response = co_await imsdk->send_message(context, [](std::shared_ptr<roc::imsdk::model::SendMessageResponse> response) {
        std::cout << "\n[message send success] id = " << response->msg->client_msg_id() << std::endl;
    });
}

inline boost::asio::awaitable<void> p_logout() {
    co_await imsdk->login_out();
}


inline boost::asio::awaitable<void> entrance() {
    std::string cmd;
    std::cout << "请输入命令: ";
    while(std::cin >> cmd) {
        if(cmd == "in") {
           co_await boost::asio::co_spawn(net_io_context, p_login(), boost::asio::use_awaitable);
        } else if(cmd == "enter") {
            co_await boost::asio::co_spawn(net_io_context, p_enter_chat(), boost::asio::use_awaitable);
        } else if(cmd == "send") {
            co_await boost::asio::co_spawn(net_io_context, p_send_message(), boost::asio::use_awaitable);
        } else if(cmd == "out") {
            co_await boost::asio::co_spawn(net_io_context, p_logout(), boost::asio::use_awaitable);
        } else if(cmd == "chats") {
            co_await boost::asio::co_spawn(net_io_context, chat_first_page(), boost::asio::use_awaitable);
        }
        std::cout << "请输入命令: ";
    }
    co_return;
}

inline void imsdk_demo() {
    boost::asio::co_spawn(main_io_context, entrance(), boost::asio::detached);
}




// ================================================
inline roc::imsdk::Config generateConfig(std::string user_id) {
    roc::imsdk::Config config;
    config.app_id = "appid_0000";
    config.user_device_id = "did_0000";
    config.user_id = user_id;
    config.user_token = "token_mock";

    return config;
}

inline roc::imsdk::model::SendMsgContext generateSendMessageContext(std::string to_user_id, std::string content) {
    roc::imsdk::model::SendMsgContext context;
    context.content = content;
    context.to_user_id = to_user_id;
    context.is_group_msg = false;
    return context;
}
// ================================================







// ================================================