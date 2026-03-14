#include "imsdk/src/core/group/controller/create/CreateGroupController.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/common/sdkwsEnum.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/conversation/ConversationManager.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/base/include/network/Error.h"
#include <memory>
#include <optional>
#include <vector>

namespace roc::imsdk::core::group {

CreateGroupController::CreateGroupController(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root(sdk_root) {}

std::optional<roc::error::Error> CreateGroupController::p_validateContext(
    CTX_T, const model::CreateGroupContext &context) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root,
                               std::optional<roc::error::Error>(roc::error::make_error(3001, "SDK root is null")))

    if (context.owner_user_id.empty()) {
        LOG_INFO("CreateGroupController", "CreateGroup validation failed: owner_user_id is empty");
        return roc::error::make_error(3002, "Create group: owner_user_id is empty");
    }
    if (context.group_name.empty()) {
        LOG_INFO("CreateGroupController", "CreateGroup validation failed: group_name is empty");
        return roc::error::make_error(3002, "Create group: group_name is empty");
    }
    return std::nullopt;
}

std::unique_ptr<network::FrontierMessage> CreateGroupController::p_buildRequest(
    CTX_T, const model::CreateGroupContext &context) {
    network::CreateGroupRequest req;
    req.set_ownerid(context.owner_user_id);
    req.set_name(context.group_name);
    for (const auto &uid : context.member_user_ids) {
        req.add_memberuids(uid);
    }
    size_t payload_size = static_cast<size_t>(req.ByteSizeLong());
    auto frontier_msg = std::make_unique<network::FrontierMessage>();
    frontier_msg->service = common::SDKWSService;
    frontier_msg->method = std::to_string(static_cast<int32_t>(common::SDKWSMethod::CREATE_GROUP));
    frontier_msg->payload.resize(payload_size);
    req.SerializeToArray(frontier_msg->payload.data(), static_cast<int>(payload_size));
    return frontier_msg;
}

std::expected<network::CreateGroupResponse, roc::error::Error> CreateGroupController::p_parseResponse(
    CTX_T, const std::vector<uint8_t> &payload) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root,
                               std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    network::CreateGroupResponse resp;
    if (!resp.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
        LOG_INFO("CreateGroupController", "CreateGroup parse response failed");
        return std::unexpected(roc::error::make_error(3007, "Create group: parse response failed"));
    }
    if (resp.errorcode() != 0) {
        LOG_INFO("CreateGroupController", "CreateGroup server error, code={}, msg={}", resp.errorcode(), resp.errormsg());
        return std::unexpected(roc::error::make_error(
            static_cast<int>(resp.errorcode()),
            resp.errormsg().empty() ? "Create group failed" : resp.errormsg()));
    }
    if (!resp.has_conversation()) {
        LOG_INFO("CreateGroupController", "CreateGroup response has no conversation");
        return std::unexpected(roc::error::make_error(3006, "Create group: response has no conversation"));
    }
    return resp;
}

boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
CreateGroupController::p_handleResult(CTX_T, const network::CreateGroupResponse &resp) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    auto conv_data = std::make_shared<network::ConversationData>(resp.conversation());
    std::vector<std::shared_ptr<network::ConversationData>> convs = {conv_data};
    auto saved = co_await sdk_root->ConversationManager()->SaveNetConversations(std::move(convs));
    if (saved.empty()) {
        LOG_INFO("CreateGroupController", "CreateGroup save conversation locally failed");
        co_return std::unexpected(roc::error::make_error(3004, "Create group: failed to save conversation locally"));
    }
    co_return saved.front();
}

boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
CreateGroupController::CreateGroup(CTX_T, const model::CreateGroupContext &context) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::unexpected(roc::error::make_error(3001, "SDK root is null")))

    /// 验证上下文
    if (auto err = p_validateContext(CTX_V, context)) {
        co_return std::unexpected(err.value());
    }

    /// 构造请求
    auto frontier_msg = p_buildRequest(CTX_V, context);
    auto response = co_await sdk_root->ConnectionManager()->SendRequest(CTX_V, std::move(frontier_msg));
    if (!response.has_value()) {
        LOG_INFO("CreateGroupController", "CreateGroup send request failed, reason: {}", response.error().to_string());
        co_return std::unexpected(response.error());
    }

    /// 解析响应
    auto parse_result = p_parseResponse(CTX_V, response.value()->payload);
    if (!parse_result.has_value()) {
        co_return std::unexpected(parse_result.error());
    }

    /// 处理结果
    co_return co_await p_handleResult(CTX_V, *parse_result);
}

} // namespace roc::imsdk::core::group
