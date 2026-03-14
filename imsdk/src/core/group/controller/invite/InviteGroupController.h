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

class InviteGroupController {
public:
    explicit InviteGroupController(std::weak_ptr<SDKRoot> sdk_root);

    /// 邀请群成员
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context);

private:
    std::optional<roc::error::Error> p_validateContext(CTX_T, const model::InviteGroupMembersContext &context);
    std::unique_ptr<network::FrontierMessage> p_buildRequest(CTX_T, const model::InviteGroupMembersContext &context);
    std::expected<network::InviteGroupMembersResponse, roc::error::Error> p_parseResponse(
        CTX_T, const std::vector<uint8_t> &payload);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::group
