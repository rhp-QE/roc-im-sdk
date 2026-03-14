#include "CmdCenter.h"
#include "core/common/logger_macro.h"
#include "core/common/macro.h"
#include "core/sdkroot/SDKRoot.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/network/connection/FrontierMessageUtility.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/network.h"

namespace roc::imsdk::core {

CmdCenter::CmdCenter(std::shared_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

void CmdCenter::AllComponentDidLoad(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    sdk_root->ConnectionManager()->AddOnPushMessageCallback([w_sdk_root = w_sdk_root, this](std::shared_ptr<const network::FrontierMessage> resp) {
        CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

        boost::asio::co_spawn(sdk_root->sdk_io_context(), this->p_handlePushMesage(resp), boost::asio::detached);
    });
}

void CmdCenter::RegistCmdHandler(CTX_T, int32_t cmd, HandlerCallbackTy handler) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    std::lock_guard<std::mutex> lock(mutex_);
    assert(handlers_.find(cmd) == handlers_.end() && "RegistCmdHandler: cmd already registered");
    handlers_[cmd] = std::move(handler);
    LOG_INFO("CmdCenter", "RegistCmdHandler: cmd {} registered", cmd);
}


// ========================  private =======================

boost::asio::awaitable<void> CmdCenter::p_handlePushMesage(std::shared_ptr<const network::FrontierMessage> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    if (resp->method != std::to_string(static_cast<int32_t>(common::SDKWSMethod::PUSH_CMD_MESSAGE))) {
        co_return;
    }

    if (resp->payload.empty()) {
        co_return;
    }

    // 直接从下推的 FrontierMessage payload 解析出 CmdMessage
    auto cmd = std::make_shared<network::CmdMessage>();
    if (!cmd->ParseFromArray(resp->payload.data(), static_cast<int>(resp->payload.size()))) {
        co_return;
    }

    uint32_t call_track_id = network::FrontierMessageUtility::ExtractTrackId(*resp).value_or(0);

    HandlerCallbackTy handler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = handlers_.find(cmd->cmd());
        if (iter == handlers_.end()) co_return;
        handler = iter->second;
    }

    co_await handler(CTX_V, cmd);
}

};