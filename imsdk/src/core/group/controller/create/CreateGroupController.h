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

namespace roc::imsdk::core::group {

class CreateGroupController {
public:
    explicit CreateGroupController(std::weak_ptr<SDKRoot> sdk_root);

    /// 创建群聊
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        CreateGroup(CTX_T, const model::CreateGroupContext &context);

private:
    std::optional<roc::error::Error> p_validateContext(CTX_T, const model::CreateGroupContext &context);
    std::unique_ptr<network::FrontierMessage> p_buildRequest(CTX_T, const model::CreateGroupContext &context);
    std::expected<network::CreateGroupResponse, roc::error::Error> p_parseResponse(
        CTX_T, const std::vector<uint8_t> &payload);
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        p_handleResult(CTX_T, const network::CreateGroupResponse &resp);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::group
