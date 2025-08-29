#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include <shared_mutex>
#include <mutex>

namespace roc::imsdk::model {

std::string ConversationModel::name() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return name_;
}

int ConversationModel::unread_count() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return unread_count_;
}

std::string ConversationModel::avatar() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return avatar_url_;
}

ConvType ConversationModel::type() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return type_;
}

int64_t ConversationModel::last_update_time() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_update_time_;
}

std::string ConversationModel::last_message_id() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_message_id_;
}

std::string ConversationModel::conversation_id() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return conversation_id_;
}

std::shared_ptr<MessageModel> ConversationModel::last_message() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_message_;
}

// Additional accessor methods
std::string ConversationModel::avatar_url() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return avatar_url_;
}

int64_t ConversationModel::last_message_time() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_message_time_;
}

std::string ConversationModel::last_message_client_id() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_message_client_id_;
}

std::string ConversationModel::last_message_server_id() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return last_message_server_id_;
}

bool ConversationModel::is_top() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_top_;
}

int64_t ConversationModel::mask() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return mask_;
}

bool ConversationModel::is_muted() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_muted_;
}

bool ConversationModel::is_deleted() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_deleted_;
}

bool ConversationModel::is_blocked() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_blocked_;
}

std::string ConversationModel::draft() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return draft_;
}

std::unordered_map<std::string, std::string> ConversationModel::sync_ext() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return sync_ext_;
}

std::unordered_map<std::string, std::string> ConversationModel::local_ext() {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return local_ext_;
}

void ConversationModel::move_from(ConversationModel&& other) noexcept {
    if (this == &other) {
        return;
    }

    // 获取写锁保护当前对象
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    // 获取源对象的写锁保护
    std::unique_lock<std::shared_mutex> other_write_lock(other.mutex_);
    
    // 移动基本类型成员
    type_ = other.type_;
    unread_count_ = other.unread_count_;
    is_top_ = other.is_top_;
    mask_ = other.mask_;
    is_muted_ = other.is_muted_;
    is_deleted_ = other.is_deleted_;
    is_blocked_ = other.is_blocked_;
    last_message_time_ = other.last_message_time_;
    last_update_time_ = other.last_update_time_;
    
    // 移动字符串成员
    name_ = std::move(other.name_);
    avatar_url_ = std::move(other.avatar_url_);
    conversation_id_ = std::move(other.conversation_id_);
    last_message_id_ = std::move(other.last_message_id_);
    last_message_client_id_ = std::move(other.last_message_client_id_);
    last_message_server_id_ = std::move(other.last_message_server_id_);
    draft_ = std::move(other.draft_);
    
    // 移动智能指针成员
    last_message_ = std::move(other.last_message_);
    
    // 移动容器成员
    sync_ext_ = std::move(other.sync_ext_);
    local_ext_ = std::move(other.local_ext_);
    
    // 重置源对象（可选，确保源对象处于有效但未定义的状态）
    other.type_ = ConvType::Single;
    other.unread_count_ = 0;
    other.is_top_ = false;
    other.mask_ = 0;
    other.is_muted_ = false;
    other.is_deleted_ = false;
    other.is_blocked_ = false;
    other.last_message_time_ = 0;
    other.last_update_time_ = 0;
    // 字符串、智能指针和容器会自动重置为空状态
}

} // namespace roc::imsdk::model 