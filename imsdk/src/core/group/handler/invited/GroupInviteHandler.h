#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::core::group {

/// 服务端下推：被邀请进群
class GroupInviteHandler {
public:
    explicit GroupInviteHandler(std::weak_ptr<SDKRoot> sdk_root);

    void AllComponentDidLoad(CTX_T);

private:
    boost::asio::awaitable<void> OnGroupInvitePush(CTX_T, std::shared_ptr<const roc::imsdk::network::CmdMessage> cmd);
    std::shared_ptr<roc::imsdk::network::ConversationData> p_parseConvDataFromCmd(CTX_T, const roc::imsdk::network::CmdMessage& cmd);

    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::group
