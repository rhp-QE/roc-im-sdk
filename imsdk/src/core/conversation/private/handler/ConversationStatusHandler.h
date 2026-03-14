#pragma once

#include "core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/uncopyable.h"
#include "network/Error.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>
#include <unordered_map>

namespace roc::imsdk::core::conversation {

class ConversationStatusHandler : public base::uncopyable {

public:

    ConversationStatusHandler(std::weak_ptr<SDKRoot> root);

    void AllComponentDidLoad();

    /// 置顶设置
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> SetTopOn(CTX_T, std::string cid, bool is_top);
    
    /// 免打扰
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> SetMute(CTX_T, std::string cid, bool is_muted);

    /// 拉黑
    boost::asio::awaitable<std::expected<bool, roc::error::Error>> SetBlock(CTX_T, std::string cid, bool is_blocked);

    /// 设置同步扩展字段
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetSyncExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &sync_ext);

    /// 设置本地扩展字段（仅本地，不发送网络请求）
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetLocalExt(CTX_T, std::string cid, const std::unordered_map<std::string, std::string> &local_ext);

    /// 删除会话
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        Delete(CTX_T, std::string cid);

private:
    std::weak_ptr<SDKRoot> w_sdk_root;

    boost::asio::awaitable<void> p_onTopOnChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);
    boost::asio::awaitable<void> p_onMuteChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);
    boost::asio::awaitable<void> p_onBlockChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);
    boost::asio::awaitable<void> p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);
    boost::asio::awaitable<void> p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);
    boost::asio::awaitable<void> p_onGroupInvite(CTX_T, std::shared_ptr<const network::CmdMessage> cmd);

    void p_registTopOnHandler();
    void p_registMuteHandler();
    void p_registBlockHandler();
    void p_registSyncExtHandler();
    void p_registDeleteHandler();
    void p_registGroupInviteHandler();

    boost::asio::awaitable<std::unique_ptr<network::CmdMessageOptResult>> p_request(CTX_T, std::unique_ptr<network::CmdMessage> cmd_msg);
     
};

} // namespace roc::imsdk::core::conversation
