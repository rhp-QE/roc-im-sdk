#include "imsdk/src/core/group/GroupManager.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/group/controller/CreateGroupController.h"
#include "imsdk/src/core/group/controller/InviteGroupController.h"
#include "imsdk/src/core/group/handler/GroupInviteHandler.h"
#include <memory>

namespace roc::imsdk::core {

GroupManager::GroupManager(std::shared_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {
    p_InitSubComponents();
}

GroupManager::~GroupManager() = default;

void GroupManager::p_InitSubComponents() {
    create_group_controller_ = std::make_unique<group::CreateGroupController>(w_sdk_root);
    invite_group_controller_ = std::make_unique<group::InviteGroupController>(w_sdk_root);
    group_invite_handler_ = std::make_unique<group::GroupInviteHandler>(w_sdk_root);
}

void GroupManager::AllComponentDidLoad(CTX_T) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root)
    group_invite_handler_->AllComponentDidLoad(CTX_V);
}

boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
GroupManager::CreateGroup(const model::CreateGroupContext &context) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await create_group_controller_->CreateGroup(CTX_V, context);
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
GroupManager::InviteGroupMembers(const model::InviteGroupMembersContext &context) {
    START_TRACK;
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))
    co_return co_await invite_group_controller_->InviteGroupMembers(CTX_V, context);
}

} // namespace roc::imsdk::core
