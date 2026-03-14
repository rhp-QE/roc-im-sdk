#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/base/include/network/Error.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/include/model/network.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>

namespace roc::imsdk::core::group {

class InviteGroupController {
public:
    explicit InviteGroupController(std::weak_ptr<SDKRoot> sdk_root);

    /// 邀请群成员
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        InviteGroupMembers(CTX_T, const model::InviteGroupMembersContext &context);

private:
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::group
