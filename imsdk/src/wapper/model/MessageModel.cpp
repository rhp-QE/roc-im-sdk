#include "imsdk/src/include/model/message/MessageModel.h"
#include <shared_mutex>
#include <mutex>

namespace roc::imsdk::model {

std::string MessageModel::content() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return content_;
}

std::string MessageModel::to_user_id() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return to_user_id_;
}

std::string MessageModel::from_user_id() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return from_user_id_;
}

std::string MessageModel::client_msg_id() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return client_msg_id_;
}

std::string MessageModel::server_msg_id() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return server_msg_id_;
}

std::string MessageModel::conversation_id() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return conversation_id_;
}

int64_t MessageModel::client_order_index() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return client_order_index_;
}

int64_t MessageModel::server_order_index() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return server_order_index_;
}

// Additional accessor methods
int MessageModel::status() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return status_;
}
    
bool MessageModel::is_pinned() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_pinned_;
}

bool MessageModel::is_deleted() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_deleted_;
}

bool MessageModel::is_recalled() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_recalled_;
}

double MessageModel::send_time() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return send_time_;
}

std::unordered_map<std::string, std::string> MessageModel::sync_ext() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return sync_ext_;
}

std::unordered_map<std::string, std::string> MessageModel::local_ext() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return local_ext_;
}

std::vector<int32_t> MessageModel::propertys() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return propertys_;
}

bool MessageModel::isGroupMessage() const {
    std::shared_lock<std::shared_mutex> read_lock(mutex_);
    return is_group_msg_;
}

void MessageModel::move_from(MessageModel&& other) noexcept {
    if (this == &other || this == nullptr) {
        return;
    }

    // 获取写锁保护当前对象
    std::unique_lock<std::shared_mutex> write_lock(mutex_);
    // 获取源对象的写锁保护
    std::unique_lock<std::shared_mutex> other_write_lock(other.mutex_);
    
    // 移动基本类型成员
    status_ = other.status_;
    is_pinned_ = other.is_pinned_;
    is_deleted_ = other.is_deleted_;
    is_recalled_ = other.is_recalled_;
    is_group_msg_ = other.is_group_msg_;
    client_order_index_ = other.client_order_index_;
    server_order_index_ = other.server_order_index_;
    send_time_ = other.send_time_;
    
    // 移动字符串成员
    content_ = std::move(other.content_);
    to_user_id_ = std::move(other.to_user_id_);
    from_user_id_ = std::move(other.from_user_id_);
    client_msg_id_ = std::move(other.client_msg_id_);
    server_msg_id_ = std::move(other.server_msg_id_);
    conversation_id_ = std::move(other.conversation_id_);
    
    // 移动容器成员
    sync_ext_ = std::move(other.sync_ext_);
    local_ext_ = std::move(other.local_ext_);
    propertys_ = std::move(other.propertys_);
    
    // 重置源对象（可选，确保源对象处于有效但未定义的状态）
    other.status_ = 0;
    other.is_pinned_ = false;
    other.is_deleted_ = false;
    other.is_recalled_ = false;
    other.is_group_msg_ = false;
    other.client_order_index_ = 0;
    other.server_order_index_ = 0;
    other.send_time_ = 0;
    // 字符串和容器会自动重置为空状态
}

} // namespace roc::imsdk::model 