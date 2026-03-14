#pragma once

#include "core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/uncopyable.h"
#include <boost/asio/awaitable.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace roc::imsdk::core {

class CmdCenter : public roc::base::uncopyable {

public: 
    using HandlerCallbackTy = std::function<boost::asio::awaitable<void>(CTX_T, std::shared_ptr<const network::CmdMessage>)>;
    CmdCenter(std::shared_ptr<SDKRoot> sdk_root);

    void AllComponentDidLoad(CTX_T);

    void RegistCmdHandler(CTX_T, int32_t cmd, HandlerCallbackTy handler);

private:

    boost::asio::awaitable<void> p_handlePushMesage(std::shared_ptr<const network::FrontierMessage> resp);

    std::weak_ptr<SDKRoot> w_sdk_root;

    std::mutex mutex_;
    std::unordered_map<int32_t, HandlerCallbackTy> handlers_;
};

}// roc::imsdk::core