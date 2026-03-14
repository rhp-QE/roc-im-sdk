#pragma once

#include "core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "core/sdkroot/SDKRoot.h"
#include "imsdk/base/include/uncopyable.h"
#include "network/Error.h"
#include <boost/asio/awaitable.hpp>
#include <expected>
#include <memory>
#include <unordered_map>
#include <vector>

namespace roc::imsdk::core::message {

class MessageStatusHandler : public base::uncopyable {

public:

    MessageStatusHandler(std::weak_ptr<SDKRoot> root);

    void AllComponentDidLoad(CTX_T);

    /// 设置消息置顶状态
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetPin(CTX_T, std::string msg_id, bool is_pinned);
    
    /// 设置消息同步扩展字段（会与现有字段合并）
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetSyncExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &sync_ext);

    /// 设置消息属性（整体替换）
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetPropertys(CTX_T, std::string msg_id, const std::vector<int32_t> &propertys);

    /// 设置消息本地扩展字段（仅本地，不发送网络请求）
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetLocalExt(CTX_T, std::string msg_id, const std::unordered_map<std::string, std::string> &local_ext);

    /// 删除消息
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        Delete(CTX_T, std::string msg_id);

    /// 撤回消息
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        Recall(CTX_T, std::string msg_id);


private:
    std::weak_ptr<SDKRoot> w_sdk_root;

    boost::asio::awaitable<void> p_onPinChange(CTX_T, std::shared_ptr<const network::CmdMessage> resp);
    boost::asio::awaitable<void> p_onSyncExtChange(CTX_T, std::shared_ptr<const network::CmdMessage> resp);
    boost::asio::awaitable<void> p_onPropertyChange(CTX_T, std::shared_ptr<const network::CmdMessage> resp);
    boost::asio::awaitable<void> p_onDelete(CTX_T, std::shared_ptr<const network::CmdMessage> resp);
    boost::asio::awaitable<void> p_onRecall(CTX_T, std::shared_ptr<const network::CmdMessage> resp);

    void p_registPinHandler(CTX_T);
    void p_registSyncExtHandler(CTX_T);
    void p_registPropertyHandler(CTX_T);
    void p_registDeleteHandler(CTX_T);
    void p_registRecallHandler(CTX_T);

    // boost::asio::awaitable<std::unique_ptr<network::CmdMessageOptResult>> p_request(CTX_T, std::unique_ptr<network::CmdMessage> cmd_msg);
     
};

}

