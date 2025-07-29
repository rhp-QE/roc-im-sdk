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

private:
    std::string content_;
    std::string from_user_id_;
    std::string to_user_id_;
    std::string conversation_id_;
};

struct MessageUpdateUnion {
    std::vector<std::shared_ptr<MessageModel>> added_messages;
    std::vector<std::shared_ptr<MessageModel>> updated_messages;
    std::vector<std::shared_ptr<MessageModel>> deleted_messages;
};


}

#endif