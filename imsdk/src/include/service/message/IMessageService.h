#ifndef ROC_IMSDK_SERVICE_MESSAGESERVICE_H
#define ROC_IMSDK_SERVICE_MESSAGESERVICE_H

#include "base/network/include/Error.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <functional>
#include <memory>

namespace roc::imsdk::model {
class MessageModel;
class MessageUpdateUnion;
}

namespace roc::imsdk {
class SDKRoot;
}

namespace roc::imsdk::service {

class MessageServiceImpl;

//================================================================
//                 Message Service Delegate
//================================================================
struct MessageServiceDelegate {
    // 消息更新
    std::function<void(const std::unique_ptr<model::MessageUpdateUnion> &update)> on_message_update;

    // 消息发送
    std::function<void(const std::unique_ptr<model::MessageUpdateUnion> &message)> on_message_sent;

    // 消息接收
    std::function<void(const std::unique_ptr<model::MessageUpdateUnion> &message)> on_message_received;

};


//================================================================
//                    Message Service
//================================================================
class IMessageService {
public:
    //================================================================
    //                    Lifecycle
    //================================================================
    IMessageService(std::weak_ptr<imsdk::SDKRoot> sdk_root);
    ~IMessageService();

    //================================================================
    //                    Delegate Management
    //================================================================
    void set_delegate(std::unique_ptr<MessageServiceDelegate> delegate);

    //================================================================
    //                    消息操作
    //================================================================
    // 发送消息
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    send_message(std::shared_ptr<model::MessageModel> message);

    // 获取消息
    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
    get_messages(const std::string &message_id);

    // 获取会话消息
    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
    get_conversation_messages(const std::string &conversation_id, const std::uint64_t &cursor, const int &limit = 50, bool forward = true);

    // 更新消息
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    update_message(const std::shared_ptr<model::MessageModel> &message);

    // 删除消息
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    delete_message(const std::shared_ptr<model::MessageModel> &message);

private:
    std::weak_ptr<imsdk::SDKRoot> sdk_root_;
    std::unique_ptr<MessageServiceImpl> impl_;
};

} // namespace roc::imsdk::service

#endif // ROC_IMSDK_SERVICE_MESSAGESERVICE_H