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
#include "imsdk/src/include/model/network.h"

namespace roc::imsdk::core {

CmdCenter::CmdCenter(std::shared_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

void CmdCenter::AllComponentDidLoad() {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)

    sdk_root->ConnectionManager()->AddOnPushMessageCallback([w_sdk_root = w_sdk_root, this](std::shared_ptr<const network::FrontierMessage> resp) {
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

boost::asio::awaitable<void> CmdCenter::p_handlePushMesage(std::shared_ptr<const network::FrontierMessage> resp) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    if (resp->method != std::to_string(static_cast<int32_t>(common::SDKWSMethod::PUSH_CMD_MESSAGE))) {
        co_return;
    }

    if (resp->payload.empty()) {
        co_return;
    }

    // 解析 BatchChangeMessagesRequest（包含 repeated CmdMessage）作为 CMD 推送的载体
    network::BatchChangeMessagesRequest cmd_request;
    if (!cmd_request.ParseFromArray(resp->payload.data(), static_cast<int>(resp->payload.size()))) {
        co_return;
    }

    uint32_t call_track_id = 0;
    auto it = resp->metadata.find("track_id");
    if (it != resp->metadata.end()) {
        call_track_id = static_cast<uint32_t>(std::stoul(it->second));
    }

    // 分发处理命令消息（从后往前 Release，保持顺序）
    while (cmd_request.cmdmessages_size() > 0) {
        std::shared_ptr<network::CmdMessage> cmd(cmd_request.mutable_cmdmessages()->ReleaseLast());

        HandlerCallbackTy handler;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto iter = handlers_.find(cmd->cmd());
            if (iter == handlers_.end()) continue;
            handler = iter->second;
        }

        co_await handler(CTX_V, cmd);
    }
}

};