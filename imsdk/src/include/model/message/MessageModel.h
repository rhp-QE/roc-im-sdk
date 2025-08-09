#ifndef ROC_IMSDK_MESSAGEMODEL_H
#define ROC_IMSDK_MESSAGEMODEL_H

#include <functional>
#include <vector>
#include <memory>

namespace roc::imsdk::model {

class ConversationModel;


class MessageModel {
public:
    MessageModel(const MessageModel &) = delete;
    MessageModel &operator=(const MessageModel &) = delete;
    MessageModel(const std::string &content, const std::string &from_user_id, const std::string &to_user_id, const std::string &conversation_id);

    std::string content();
    std::string to_user_id();
    std::string from_user_id();
    std::string conversation_id();
    bool isGroupMessage();
    int64_t client_order_index();
    int64_t server_order_index();

private:
    std::string content_;
    std::string from_user_id_;
    std::string to_user_id_;
    std::string conversation_id_;
    int64_t client_order_index_;
    int64_t server_order_index_;
};


enum class MessageUpdateReson {
    DELETE,
    UPDATE,
};


struct SendMsgContext {
    std::string content;
    std::string sync_ext;
    std::string local_ext;

    bool is_group_msg;
    std::string conv_id;
    std::string to_user_id;
};


struct SendMessageResponse {
    bool is_success;
    std::string error_msg;
    std::shared_ptr<MessageModel> msg;
};


struct QueryConvMessagesResult {
    std::vector<std::shared_ptr<MessageModel>> messages;
    int64_t cursor;
    bool has_more;
};


struct ReceiveMessagesResult {
    // 会话id -> 消息列表
    std::unordered_map<std::string, std::vector<std::shared_ptr<const MessageModel>>> msgs;
    // 会话id -> 会话信息
    std::unordered_map<std::string, std::shared_ptr<const ConversationModel>> convs;
};


// callback -------------
using OnMessageUpdateCallbackType = std::function<void(std::shared_ptr<const MessageModel> msg, MessageUpdateReson reason)>;
using OnReceiveMessagesCallbackType = std::function<void(ReceiveMessagesResult result)>;
// ------------------------

}

#endif