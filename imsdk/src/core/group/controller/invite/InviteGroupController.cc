#include "imsdk/src/core/group/controller/invite/InviteGroupController.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"
#include <memory>
#include <optional>
#include <vector>

namespace roc::imsdk::core::group {

InviteGroupController::InviteGroupController(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

std::optional<roc::error::Error> InviteGroupController::p_validateContext(
    CTX_T, const model::InviteGroupMembersContext &context) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root,
                               std::optional<roc::error::Error>(roc::error::make_error(3001, "SDK root is null")))

    if (context.conv_id.empty()) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers validation failed: conv_id is empty");
        return roc::error::make_error(3002, "Invite group members: conv_id is empty");
    }
    if (context.member_user_ids.empty()) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers validation failed: member_user_ids is empty");
        return roc::error::make_error(3002, "Invite group members: member_user_ids is empty");
    }
    return std::nullopt;
}

std::unique_ptr<network::FrontierMessage> InviteGroupController::p_buildRequest(
    CTX_T, const model::InviteGroupMembersContext &context) {
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
    return frontier_msg;
}

std::expected<network::InviteGroupMembersResponse, roc::error::Error> InviteGroupController::p_parseResponse(
    CTX_T, const std::vector<uint8_t> &payload) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root,
                               std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::InviteGroupMembersResponse resp;
    if (!resp.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers parse response failed");
        return std::unexpected(roc::error::make_error(3008, "Invite group members: parse response failed"));
    }
    if (resp.errorcode() != 0) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers server error, code={}, msg={}", resp.errorcode(), resp.errormsg());
        return std::unexpected(roc::error::make_error(
            static_cast<int>(resp.errorcode()),
            resp.errormsg().empty() ? "Invite group members failed" : resp.errormsg()));
    }
    return resp;
}

boost::asio::awaitable<std::expected<bool, roc::error::Error>>
InviteGroupController::InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    // 前置数据校验
    if (auto err = p_validateContext(CTX_V, context)) {
        co_return std::unexpected(err.value());
    }

    // 构造请求
    auto frontier_msg = p_buildRequest(CTX_V, context);
    
    // 发起请求
    auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!response.has_value()) {
        LOG_INFO("InviteGroupController", "InviteGroupMembers send request failed, reason: {}", response.error().to_string());
        co_return std::unexpected(response.error());
    }

    // 解析响应
    auto parse_result = p_parseResponse(CTX_V, response.value()->payload);
    if (!parse_result.has_value()) {
        co_return std::unexpected(parse_result.error());
    }

    // 成功
    co_return true;
}

} // namespace roc::imsdk::core::group
