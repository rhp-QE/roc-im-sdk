#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/network.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>
#include <optional>
#include <vector>

namespace roc::imsdk::core {

class GroupManager : public roc::base::uncopyable {
public:
    GroupManager(std::shared_ptr<SDKRoot> sdk_root);
    ~GroupManager();

    void AllComponentDidLoad();

    /// 创建群聊
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        CreateGroup(const model::CreateGroupContext &context);

    /// 邀请群成员
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        InviteGroupMembers(const model::InviteGroupMembersContext &context);

private:
    std::optional<roc::error::Error> p_validateCreateGroupContext(CTX_T, const model::CreateGroupContext &context);
    std::unique_ptr<network::FrontierMessage> p_buildCreateGroupRequest(CTX_T, const model::CreateGroupContext &context);
    std::expected<network::CreateGroupResponse, roc::error::Error> p_parseCreateGroupResponse(
        CTX_T, const std::vector<uint8_t> &payload);
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        p_handleCreateGroupResult(CTX_T, const network::CreateGroupResponse &resp);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core