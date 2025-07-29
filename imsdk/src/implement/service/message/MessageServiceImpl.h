#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/service/message/IMessageService.h"

#include "imsdk/src/core/service/message/MessageSendLogic.h"
#include "imsdk/src/core/service/message/MessageCacheLogic.h"

#include <memory.h>

namespace roc::imsdk::service {

class MessageServiceImpl {
public:
    MessageServiceImpl(std::weak_ptr<imsdk::SDKRoot> sdk_root);
    ~MessageServiceImpl();

    void set_delegate(std::unique_ptr<MessageServiceDelegate> delegate);

    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    send_message(const std::shared_ptr<model::MessageModel>& message);

    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
    get_messages(const std::string& message_id);

    boost::asio::awaitable<std::expected<std::vector<std::shared_ptr<model::MessageModel>>, roc::error::Error>>
    get_conversation_messages(const std::string& conversation_id, const std::uint64_t& cursor, const int& limit, bool forward);

    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    update_message(const std::shared_ptr<model::MessageModel>& message);

    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
    delete_message(const std::shared_ptr<model::MessageModel>& message);

private:
    std::weak_ptr<imsdk::SDKRoot> sdk_root_;
    std::unique_ptr<MessageServiceDelegate> delegate_;

    std::unique_ptr<MessageCacheLogic> message_cache_logic_;
    std::unique_ptr<MessageSendLogic> message_send_logic_;
};

} // namespace roc::imsdk::service