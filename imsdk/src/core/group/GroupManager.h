#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/include/model/network.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>

namespace roc::imsdk::core::group {
class CreateGroupController;
class InviteGroupController;
class GroupInviteHandler;
}

namespace roc::imsdk::core {

class GroupManager : public roc::base::uncopyable {
public:
    GroupManager(std::shared_ptr<SDKRoot> sdk_root);
    ~GroupManager();

    void AllComponentDidLoad(CTX_T);

    /// 创建群聊
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        CreateGroup(const model::CreateGroupContext &context);

    /// 邀请群成员
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        InviteGroupMembers(const model::InviteGroupMembersContext &context);

private:
    void p_InitSubComponents();

    std::unique_ptr<group::CreateGroupController> create_group_controller_;
    std::unique_ptr<group::InviteGroupController> invite_group_controller_;
    std::unique_ptr<group::GroupInviteHandler> group_invite_handler_;
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core
