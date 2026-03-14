#include "imsdk/src/core/group/controller/InviteGroupController.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"
#include <memory>

namespace roc::imsdk::core::group {

InviteGroupController::InviteGroupController(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
InviteGroupController::InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::InviteGroupMembersRequest req;
    req.set_convid(context.conv_id);
    for (const auto &uid : context.member_user_ids) {
        req.add_memberuids(uid);
    }

    size_t payload_size = static_cast<size_t>(req.ByteSizeLong());
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::INVITE_GROUP_MEMBERS));
    frontier_msg->payload.resize(payload_size);
    req.SerializeToArray(frontier_msg->payload.data(), static_cast<int>(payload_size));

    auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!response.has_value()) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers send request failed, reason: {}", response.error().to_string());
        co_return std::unexpected(response.error());
    }

    network::InviteGroupMembersResponse resp;
    if (!resp.ParseFromArray(response.value()->payload.data(), static_cast<int>(response.value()->payload.size()))) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers parse response failed");
        co_return std::unexpected(roc::error::make_error(3008, "Invite group members: parse response failed"));
    }
    if (resp.errorcode() != 0) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers server error, code={}, msg={}", resp.errorcode(), resp.errormsg());
        co_return std::unexpected(roc::error::make_error(
            static_cast<int>(resp.errorcode()),
            resp.errormsg().empty() ? "Invite group members failed" : resp.errormsg()));
    }
    co_return true;
}

} // namespace roc::imsdk::core::group
