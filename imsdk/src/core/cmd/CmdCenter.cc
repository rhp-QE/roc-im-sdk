#include "CmdCenter.h"
#include "core/common/macro.h"
#include "core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"

namespace roc::imsdk::core {

CmdCenter::CmdCenter(std::shared_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

void CmdCenter::AllComponentDidLoad() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    sdk_root->ConnectionManager()->AddOnPushMessageCallback([w_sdk_root = w_sdk_root, this](std::shared_ptr<const network::SdkWSResp> resp) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

        boost::asio::co_spawn(sdk_root->sdk_io_context(), this->p_handlePushMesage(resp), boost::asio::detached);
    });
}

void CmdCenter::RegistCmdHandler(int32_t cmd, HandlerCallbackTy handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (handlers_.find(cmd) != handlers_.end()) {
        handlers_[cmd] = handler;
    }
}


// ========================  private =======================

boost::asio::awaitable<void> CmdCenter::p_handlePushMesage(std::shared_ptr<const network::SdkWSResp> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    bool can_not_handle = false;
    can_not_handle = can_not_handle || (resp->service() != common::SDKWSService);
    can_not_handle = can_not_handle || (resp->method()  != static_cast<int32_t>(common::SDKWSMethod::PUSH_CMD_MESSAGE));
    if (can_not_handle) {
        // 转发给 业务层处理
        co_return;
    }

    // 前置检查
    if (resp->method() != static_cast<int32_t>(common::SDKWSMethod::PUSH_CMD_MESSAGE)) {
        co_return;
    }

    std::shared_ptr<network::CmdMessageArray> cmd_message_array = std::make_unique<network::CmdMessageArray>();
    if (!cmd_message_array->ParseFromString(resp->data())) {
        co_return;
    }

    uint32_t call_track_id = resp->trackid();

    // 分发处理命令消息
    while(!cmd_message_array->cmdmsgs().empty()) {
        std::shared_ptr<network::CmdMessage> cmd = std::shared_ptr<network::CmdMessage>(cmd_message_array->mutable_cmdmsgs()->ReleaseLast());

        // 检擦连续性 TODO

        HandlerCallbackTy handler;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            handler = handlers_[cmd->cmd()];
        }

        /// 线性处理
        co_await handler(CTX_V, cmd);
    }
}

};