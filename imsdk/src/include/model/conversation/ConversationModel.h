#ifndef ROC_IMSDK_MODEL_CONVERSATIONMODEL_H
#define ROC_IMSDK_MODEL_CONVERSATIONMODEL_H

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <mutex>


namespace roc::imsdk::core {
    class ConversationManager;
}

namespace roc::imsdk::core::conversation {
    class Convert;
    class ConvDatasource;
    class ConversationStatusHandler;
}

namespace roc::imsdk::model {

class MessageModel;

enum class ConvType {
    Single,
    Group
};


class ConversationModel {
public:
    // 线程安全的访问器方法
    std::string name();
    int unread_count();
    std::string avatar();
    ConvType type();
    int64_t last_update_time();
    std::string last_message_id();
    std::string conversation_id();
    std::shared_ptr<MessageModel> last_message();

    // Additional accessor methods
    std::string avatar_url();
    int64_t last_message_time();
    std::string last_message_client_id();
    std::string last_message_server_id();
    bool is_top();
    int64_t mask();
    bool is_muted();
    bool is_deleted();
    bool is_blocked();
    std::string draft();
    std::unordered_map<std::string, std::string> sync_ext();
    std::unordered_map<std::string, std::string> local_ext();

    std::vector<std::string> members();

    friend class roc::imsdk::core::ConversationManager;
    friend class roc::imsdk::core::conversation::Convert;
    friend class roc::imsdk::core::conversation::ConvDatasource;
    friend class roc::imsdk::core::conversation::ConversationStatusHandler;

private:
    // 私有移动操作 - 绕过系统移动赋值，手动实现数据移动
    void move_from(ConversationModel&& other) noexcept;
    
    // 线程安全保护
    mutable std::shared_mutex mutex_;
    ConvType type_;
    
    std::string name_;
    
    int unread_count_ = 0;
    
    std::string avatar_url_;
    
    int64_t last_message_time_;
    
    int64_t last_update_time_;
    
    std::string conversation_id_;
    
    std::string last_message_id_;
    
    std::string last_message_client_id_;
    
    std::string last_message_server_id_;
    
    std::shared_ptr<MessageModel> last_message_;

    std::vector<std::string> members_;

    bool is_top_;
    
    int64_t mask_;
    
    bool is_muted_;
    
    bool is_deleted_;
    
    bool is_blocked_;
    
    std::string draft_;

    std::unordered_map<std::string, std::string> sync_ext_;
    
    std::unordered_map<std::string, std::string> local_ext_;

    void set_top(bool is_top) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        is_top_ = is_top;
    }

    void set_mute(bool is_mute) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        is_muted_ = is_mute;
    }

    void set_block(bool is_block) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        is_blocked_ = is_block;
    }

    void set_sync_ext(const std::unordered_map<std::string, std::string> &sync_ext) {
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        sync_ext_ = sync_ext;
    }
};



struct OnConversationResult {
    /// 新增的会话
    std::vector<std::shared_ptr<ConversationModel>> added_convs;

    /// 更新的会话
    std::vector<std::shared_ptr<ConversationModel>> updated_convs;

    /// 删除的会话
    std::vector<std::shared_ptr<ConversationModel>> deleted_convs;

    /// 置顶状态变更的会话
    std::vector<std::shared_ptr<ConversationModel>> top_on_change_convs;

    /// 拉黑状态变更的会话
    std::vector<std::shared_ptr<ConversationModel>> block_change_convs;

    /// 禁言状态变更的会话
    std::vector<std::shared_ptr<ConversationModel>> mute_change_convs;

    /// sync_ext状态变更的会话
    std::vector<std::shared_ptr<ConversationModel>> sync_ext_change_convs;
};



struct LoadUserConvsResult {
    std::vector<std::shared_ptr<ConversationModel>> convs;
    int64_t cursor;
    bool has_more;
};


// callback -------------
using OnConvUpdateCallbackType = std::function<void(std::shared_ptr<OnConversationResult> result)>;
// ------------------------

} // namespace roc::imsdk::model

#endif // ROC_IMSDK_MODEL_CONVERSATIONMODEL_H 