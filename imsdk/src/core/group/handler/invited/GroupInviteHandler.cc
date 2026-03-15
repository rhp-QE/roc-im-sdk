#include "imsdk/src/core/group/handler/invited/GroupInviteHandler.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/core/cmd/CmdCenter.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::core::group {

GroupInviteHandler::GroupInviteHandler(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

std::shared_ptr<roc::imsdk::network::ConversationData> GroupInviteHandler::p_parseConvDataFromCmd(
    CTX_T, const roc::imsdk::network::CmdMessage& cmd) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr)

    if (cmd.data().empty()) {
        LOG_INFO("GroupInviteHandler", "p_parseConvDataFromCmd: data empty");
        return nullptr;
    }
    auto conv = std::make_shared<roc::imsdk::network::ConversationData>();
    if (!conv->ParseFromArray(cmd.data().data(), static_cast<int>(cmd.data().size()))) {
        LOG_INFO("GroupInviteHandler", "p_parseConvDataFromCmd: ParseFromArray failed");
        return nullptr;
    }
    return conv;
}

void GroupInviteHandler::AllComponentDidLoad(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    sdk_root->cmd_center()->RegistCmdHandler(
        CTX_V,
        static_cast<int32_t>(common::CmdMessageOp::CONV_GROUP_INVITED),
        [w_sdk_root = w_sdk_root, this](CTX_T, std::shared_ptr<const network::CmdMessage> cmd) -> boost::asio::awaitable<void> {
            CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)
            co_await this->OnGroupInvitePush(CTX_V, cmd);
        });
}

boost::asio::awaitable<void> GroupInviteHandler::OnGroupInvitePush(CTX_T, std::shared_ptr<const roc::imsdk::network::CmdMessage> cmd) {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root)

    // 数据解析
    auto net_conv = p_parseConvDataFromCmd(CTX_V, *cmd);
    if (!net_conv) {
        co_return;
    }

    // 保存会话
    auto conv_manager = sdk_root->ConversationManager();
    auto sdk_convs = co_await conv_manager->SaveNetConversations({net_conv});

    // 上抛
    if (!sdk_convs.empty()) {
        auto on_conversation_result = std::make_shared<model::OnConversationResult>();
        on_conversation_result->insert_convs.push_back(sdk_convs[0]);
        on_conversation_result->invited_group_convs.push_back(sdk_convs[0]);
        base::util::safe_invoke_block(conv_manager->OnConversationsCallback(), on_conversation_result);
    }
}

} // namespace roc::imsdk::core::group
