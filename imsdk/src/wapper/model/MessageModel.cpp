#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::model {

std::string MessageModel::content() const {
    return content_;
}

std::string MessageModel::to_user_id() const {
    return to_user_id_;
}

std::string MessageModel::from_user_id() const {
    return from_user_id_;
}

std::string MessageModel::client_msg_id() const {
    return client_msg_id_;
}

std::string MessageModel::server_msg_id() const {
    return server_msg_id_;
}

std::string MessageModel::conversation_id() const {
    return conversation_id_;
}

int64_t MessageModel::client_order_index() const {
    return client_order_index_;
}

int64_t MessageModel::server_order_index() const {
    return server_order_index_;
}

// Additional accessor methods
int MessageModel::status() const {
    return status_;
}
    
bool MessageModel::is_pinned() const {
    return is_pinned_;
}

bool MessageModel::is_deleted() const {
    return is_deleted_;
}

bool MessageModel::is_recalled() const {
    return is_recalled_;
}

double MessageModel::send_time() const {
    return send_time_;
}

std::unordered_map<std::string, std::string> MessageModel::sync_ext() const {
    return sync_ext_;
}

std::unordered_map<std::string, std::string> MessageModel::local_ext() const {
    return local_ext_;
}

bool MessageModel::isGroupMessage() const {
    return is_group_msg_;
}

} // namespace roc::imsdk::model 