#ifndef ROC_IMSDK_INCLUDE_IMSDK_H
#define ROC_IMSDK_INCLUDE_IMSDK_H

#include <boost/asio/awaitable.hpp>
#include "base/Uncopyable.h"
#include <memory>
namespace roc::imsdk {

    class SDKRoot;
    class Config;

namespace service {
    class IMessageService;
    class IConversationService;
}

class IMSDK : public std::enable_shared_from_this<IMSDK>,
              public  roc::base::uncopyable
{

public:
    IMSDK();
    ~IMSDK();

    // 初始化SDK
    boost::asio::awaitable<bool> init_sdk(const Config config);

    // 获取消息服务
    service::IMessageService* msg_service();

    // 获取会话服务
    service::IConversationService* conv_service();

private:
    std::shared_ptr<SDKRoot> sdk_root_;
};

} // namespace roc::imsdk

#endif // ROC_IMSDK_INCLUDE_IMSDK_H