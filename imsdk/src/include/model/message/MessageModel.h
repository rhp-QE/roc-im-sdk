#ifndef ROC_IMSDK_MESSAGEMODEL_H
#define ROC_IMSDK_MESSAGEMODEL_H

#include <vector>
#include <memory>

namespace roc::imsdk::model {

class MessageModel {
public:
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
    MSG_NEW,
    MSG_DELETE,
    MSG_UPDATE,
};


}

#endif