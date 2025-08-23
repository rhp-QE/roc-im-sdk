#include "imsdk/src/include/model/message/MessageModel.h"

namespace roc::imsdk::model {

std::string MessageModel::content() {
    return content_;
}

std::string MessageModel::to_user_id() {
    return to_user_id_;
}

std::string MessageModel::from_user_id() {
    return from_user_id_;
}

std::string MessageModel::client_msg_id() {
    return client_msg_id_;
}

std::string MessageModel::server_msg_id() {
    return server_msg_id_;
}

std::string MessageModel::conversation_id() {
    return conversation_id_;
}

int64_t MessageModel::client_order_index() {
    return client_order_index_;
}

int64_t MessageModel::server_order_index() {
    return server_order_index_;
}

// Additional accessor methods
int MessageModel::status() {
    return status_;
}

bool MessageModel::is_pinned() {
    return is_pinned_;
}

bool MessageModel::is_deleted() {
    return is_deleted_;
}

bool MessageModel::is_recalled() {
    return is_recalled_;
}

double MessageModel::send_time() {
    return send_time_;
}

std::unordered_map<std::string, std::string> MessageModel::sync_ext() {
    return sync_ext_;
}

std::unordered_map<std::string, std::string> MessageModel::local_ext() {
    return local_ext_;
}

bool MessageModel::isGroupMessage() {
    return is_group_msg_;
}

} // namespace roc::imsdk::model 