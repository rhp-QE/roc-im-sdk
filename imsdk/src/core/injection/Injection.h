#pragma once

#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include <functional>
#include <memory>

namespace roc::imsdk::injection {


struct ConvMessagesUnion {
    std::shared_ptr<imsdk::model::ConversationModel> sdk_conv;
    std::vector<std::shared_ptr<imsdk::model::MessageModel>> sdk_msgs;
};


// 会话排序规则
using ConvSortRuleType = std::function<bool(const std::shared_ptr<model::ConversationModel> &a, const std::shared_ptr<model::ConversationModel> &b)>;

// 消息排序规则
using MsgSortRuleType = std::function<bool(const std::shared_ptr<model::MessageModel> &a, const std::shared_ptr<model::MessageModel> &b)>;

// 接收到新消息
using OnNewMessageCallback = std::function<void(std::vector<ConvMessagesUnion> conv_messages_union)>;

// 消息更新
using OnMsgUpdateCallback = std::function<void(std::vector<std::shared_ptr<model::MessageModel>> messages)>;

// 会话更新
using OnConvUpdateCallback = std::function<void(std::vector<std::shared_ptr<model::ConversationModel>> conversations)>;


//================================================================
//                    Injection
//================================================================
class Injection {

public:
    Injection();
    ~Injection();

    // 会话排序规则
    ConvSortRuleType conv_sort_rule;

    // 消息排序规则
    MsgSortRuleType msg_sort_rule;

    // 接收到新消息
    OnNewMessageCallback on_new_message_callback;

    // 消息更新
    OnMsgUpdateCallback on_msg_update_callback;

    // 会话更新
    OnConvUpdateCallback on_conv_update_callback;
};

} // namespace roc::imsdk::injection
