#ifndef ROC_IMSDK_MESSAGEMODEL_H
#define ROC_IMSDK_MESSAGEMODEL_H

#include <functional>
#include <vector>
#include <memory>
#include <shared_mutex>
#include "base/Uncopyable.h"

namespace roc::imsdk::core {
    class MessageManager;
}

namespace roc::imsdk::core::message {
    class Convert;
}

namespace roc::imsdk::model {

class ConversationModel;


class MessageModel : public roc::base::uncopyable {
public:
    // 线程安全的访问器方法
    std::string content() const;
    std::string to_user_id() const;
    std::string from_user_id() const;
    std::string client_msg_id() const;
    std::string server_msg_id() const;
    std::string conversation_id() const;
    int64_t client_order_index() const;
    int64_t server_order_index() const;

    bool isGroupMessage() const;
    
    // Additional accessor methods
    int status() const;
    bool is_pinned() const;
    bool is_deleted() const;
    bool is_recalled() const;
    double send_time() const;
    std::unordered_map<std::string, std::string> sync_ext() const;
    std::unordered_map<std::string, std::string> local_ext() const;

    friend class roc::imsdk::core::message::Convert;

private:
    // 私有移动操作 - 绕过系统移动赋值，手动实现数据移动
    void move_from(MessageModel&& other) noexcept;
    
    // 线程安全保护
    mutable std::shared_mutex mutex_;

    int  status_;
    
    bool is_pinned_;
    
    bool is_deleted_;
    
    bool is_recalled_;

    bool is_group_msg_;

    std::string content_;

    std::string to_user_id_; 
    
    std::string from_user_id_;

    std::string client_msg_id_;
    
    std::string server_msg_id_;

    std::string conversation_id_;

    int64_t client_order_index_;
    
    int64_t server_order_index_;

    int64_t send_time_;

    std::unordered_map<std::string, std::string>  sync_ext_;
    
    std::unordered_map<std::string, std::string>  local_ext_;

};


enum class MessageUpdateReson {
    DELETE,   // 删除消息
    UPDATE,   // 更新消息
    RECALL,   // 撤回消息
    OFFLINE,  // 离线消息
    DB_EMPTY, // 因BD为空而补齐的消息
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


struct LoadConvMessagesResult {
    std::vector<std::shared_ptr<MessageModel>> messages;
    int64_t cursor;
    bool has_more;
};


struct OnMessageResult {
    /// 删除的消息
    std::vector<std::shared_ptr<const MessageModel>> deleted_msgs;

    /// 撤回的消息
    std::vector<std::shared_ptr<const MessageModel>> recalled_msgs;

    /// 更新的消息
    std::vector<std::shared_ptr<const MessageModel>> updated_msgs; 

    /// 实时消息 (在线收到的消息)
    std::vector<std::shared_ptr<const MessageModel>> real_time_msgs;

    /// 离线消息 (本设备离线状态下且没有被别的设备接收过的消息)
    std::vector<std::shared_ptr<const MessageModel>> offline_not_received_msgs;

    /// 空洞消息 (本设备离线状态下被别的设备接收过的消息 或 因本地数据库损坏而补齐的消息)
    std::vector<std::shared_ptr<const MessageModel>> offline_received_msgs;
    
    // 会话id -> 会话信息
    std::unordered_map<std::string, std::shared_ptr<const ConversationModel>> convs;
};


// callback -------------
using OnMessagesCallbackType = std::function<void(OnMessageResult result)>;
// ------------------------

}

#endif