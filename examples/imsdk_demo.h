#pragma once

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include "im/base/coroutine.h"
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/include/config.h"
#include "BaseConfig.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/base/include/utils/utils.h"
#include "adapter/log/SpdlogAdapter.h"

int64_t message_cursor = -1;
int64_t conv_cursor = -1;

// ==================
std::vector<std::thread> threads;

std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> demo_net_io_context_work;
std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> demo_sdk_io_context_work;

std::shared_ptr<boost::asio::io_context> demo_net_io_context;
std::shared_ptr<boost::asio::io_context> demo_sdk_io_context;

std::shared_ptr<roc::imsdk::SpdlogAdapter> spdlog_adapter;
// ==================


inline roc::imsdk::Config generateConfig(std::string user_id);
inline roc::imsdk::model::SendMsgContext generateSendMessageContext(std::string to_user_id, std::string content);

std::shared_ptr<roc::imsdk::IMSDK> imsdk;

inline void print_msg(std::shared_ptr<roc::imsdk::model::MessageModel> msg) {
    std::cout << "      ----------------------------------------" << std::endl;
    std::cout << "      [message_id:   ] " << msg->client_msg_id() << std::endl;
    std::cout << "      [message_order:] " << msg->client_order_index() << std::endl;
    std::cout << "      [content:      ] " << msg->content() << std::endl;
    std::cout << "      [from:         ] " << msg->from_user_id() << std::endl;
    std::cout << "      [to:           ] " << msg->to_user_id() << std::endl;
    std::cout << "      ----------------------------------------" << std::endl;
}

inline void print_conv(std::shared_ptr<roc::imsdk::model::ConversationModel> conv) {
    std::cout << "      ----------------------------------------" << std::endl;
    std::cout << "      [conversation_id: ] " << conv->conversation_id() << std::endl;
    std::cout << "      [conversation_name: ] " << conv->name() << std::endl;
    std::cout << "      [conversation_type: ] " << (conv->type() == roc::imsdk::model::ConvType::Single ? "单聊" : "群聊") << std::endl;
    std::cout << "      ----------------------------------------" << std::endl;
}

inline boost::asio::awaitable<bool> p_login() {
    std::cout << "请输入用户ID: ";
    std::string user_id;
    std::cin >> user_id;

    // 创建 spdlog 适配器
    spdlog_adapter = roc::imsdk::SpdlogAdapter::create(user_id+"_log");
    spdlog_adapter->start_periodic_flush_async(demo_sdk_io_context, 1000);

    auto config = generateConfig(user_id);

    // 创建sdk
    imsdk = std::make_shared<roc::imsdk::IMSDK>();

    // 注入日志器
    imsdk->InjectLogger(spdlog_adapter->get_logger());

    // 初始化sdk
    co_await imsdk->InitSdk(config);

    // 监听长链状态
    imsdk->OnNetworkStatusChange([](roc::imsdk::network::NetworkStatus status) {
        std::cout << "长链状态: " << (status == roc::imsdk::network::NetworkStatus::NETWORK_STATUS_CONNECTED ? "连接" : "断开") << std::endl;
    });

    // 监听消息更新
    imsdk->OnMessagee([](roc::imsdk::model::OnMessageResult result) {
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

    // 监听会话更新
    imsdk->OnConvUpdate([](std::shared_ptr<roc::imsdk::model::OnConversationResult> result) {
        std::cout << "\n ==============会话更新 (begin) =============" << std::endl;
        std::cout << " 新增会话: " << result->new_convs.size() << std::endl;
        for (auto conv : result->new_convs) {
            print_conv(conv);
        }
    });

    // 运行sdk
    bool res = co_await imsdk->run();
    if (res) {
        std::cout << "登录成功" << std::endl;
    } else {
        std::cout << "登录失败" << std::endl;
        co_await imsdk->LoginOut();
    }
    co_return res;
}

inline boost::asio::awaitable<void> chat_first_page() {
    std::cout << "登录成功 正在拉取会话列表...." << std::endl;
    auto convs = co_await imsdk->ConvsWhenLogin();
    conv_cursor = convs->cursor;

    std::cout<<"\n========首屏会话 (begin) ========"<<std::endl;
    // 打印会话信息
    for(auto conv : convs->convs) {
        std::cout << "      [conv_id: ] " << conv->conversation_id() << std::endl;
    }
    std::cout<<"========首屏会话 (end) ========\n"<<std::endl;
}

inline boost::asio::awaitable<void> p_login_out() {
    co_await imsdk->LoginOut();
}

inline boost::asio::awaitable<void> p_enter_chat() {
    std::cout << "请输入会话ID: ";
    std::string conv_id;
    std::cin >> conv_id;

    auto messages = co_await imsdk->MessagesWhenEnterChat(conv_id);
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
    auto response = co_await imsdk->SendMessage(context, [](std::shared_ptr<roc::imsdk::model::SendMessageResponse> response) {
        if (response->error_code) {
            std::cout<<"\n消息发送失败: " << response->error_msg << "\n" <<std::endl;
            return;
        }
        std::cout << "\n[message send success] id = " << response->msg->client_msg_id() << std::endl;
    });
}

inline boost::asio::awaitable<void> p_logout() {
    co_await imsdk->LoginOut();
}

/// 创建群聊测试
inline boost::asio::awaitable<void> p_create_group() {
    std::cout << "请输入群主ID (owner_user_id): ";
    std::string owner_id;
    std::cin >> owner_id;
    std::cout << "请输入群名称 (group_name): ";
    std::string group_name;
    std::cin >> group_name;
    std::cout << "请输入邀请的进群人数: ";
    size_t invite_count = 0;
    std::cin >> invite_count;
    std::vector<std::string> member_ids;
    for (size_t i = 0; i < invite_count; ++i) {
        std::cout << "请输入第 " << (i + 1) << " 个成员ID: ";
        std::string id;
        std::cin >> id;
        member_ids.push_back(id);
    }

    roc::imsdk::model::CreateGroupContext ctx;
    ctx.owner_user_id = owner_id;
    ctx.group_name = group_name;
    ctx.member_user_ids = member_ids;

    auto result = co_await imsdk->CreateGroup(ctx);
    if (!result.has_value()) {
        std::cout << "创建群聊失败: " << result.error().to_string() << std::endl;
        co_return;
    }
    std::cout << "创建群聊成功, 会话ID: " << (*result)->conversation_id() << std::endl;
}

/// 邀请进群测试
inline boost::asio::awaitable<void> p_invite_group_members() {
    std::cout << "请输入会话ID (conv_id): ";
    std::string conv_id;
    std::cin >> conv_id;
    std::cout << "请输入邀请的进群人数: ";
    size_t invite_count = 0;
    std::cin >> invite_count;
    std::vector<std::string> member_ids;
    for (size_t i = 0; i < invite_count; ++i) {
        std::cout << "请输入第 " << (i + 1) << " 个成员ID: ";
        std::string id;
        std::cin >> id;
        member_ids.push_back(id);
    }

    roc::imsdk::model::InviteGroupMembersContext ctx;
    ctx.conv_id = conv_id;
    ctx.member_user_ids = member_ids;

    auto result = co_await imsdk->InviteGroupMembers(ctx);
    if (!result.has_value()) {
        std::cout << "邀请进群失败: " << result.error().to_string() << std::endl;
        co_return;
    }
    std::cout << "邀请进群成功" << std::endl;
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
        } else if(cmd == "cg" || cmd == "create_group") {
            co_await boost::asio::co_spawn(net_io_context, p_create_group(), boost::asio::use_awaitable);
        } else if(cmd == "invite") {
            co_await boost::asio::co_spawn(net_io_context, p_invite_group_members(), boost::asio::use_awaitable);
        }
        std::cout << "请输入命令: ";
    }
    co_return;
}

inline void imsdk_demo() {

    demo_net_io_context = std::make_shared<boost::asio::io_context>();
    demo_sdk_io_context = std::make_shared<boost::asio::io_context>();

    demo_net_io_context_work = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(*demo_net_io_context));
    demo_sdk_io_context_work = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(*demo_sdk_io_context));

    threads.push_back(std::thread([&]() {
        demo_net_io_context->run();
    }));

    for (int i = 0; i < 3; ++i) {
        threads.push_back(std::thread([&]() {
            demo_sdk_io_context->run();
        }));
    }

    boost::asio::co_spawn(demo_sdk_io_context->get_executor(), entrance(), boost::asio::detached);
}




// ================================================
inline roc::imsdk::Config generateConfig(std::string user_id) {
    roc::imsdk::Config config;
    config.app_id = "appid_0000";
    config.user_device_id = "did_0000";
    config.user_id = user_id;
    config.user_token = "token_mock";

    config.net_io_context = demo_net_io_context;
    config.sdk_io_context = demo_sdk_io_context;

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