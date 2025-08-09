#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/message/db_model/MessageORM.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <memory>
#include <unordered_map>
#include <vector>

// Forward declaration
namespace roc::imsdk::core {
    class MessageManager;
}

namespace roc::imsdk::core::message {

class SaveMessage {
public:
    /// 保存网络消息
    static std::vector<std::shared_ptr<model::MessageModel>> save_net_msgs(W_SDK_ROOT, std::vector<const network::MsgData *> msgs);
    
    /// 设置 SDK 消息
    static void set_sdk_msg(W_SDK_ROOT, const core::message::MessageORM *msg);
    
    /// 根据 ID 获取 SDK 消息
    static std::shared_ptr<model::MessageModel> sdk_msg_for_id(W_SDK_ROOT, const std::string &msg_id);
    
    /// 获取会话的缺失消息区间
    static std::vector<std::pair<int64_t, int64_t>> empty_message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id);
    
    /// 生成客户端消息 ID
    static std::string generate_client_msg_id();

private:
    /// 更新消息区间
    static void update_message_range_for_message(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs);

    /// 获取会话的消息区间
    static std::vector<std::pair<int64_t, int64_t>> message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id);
};

} // namespace roc::imsdk::core::message
