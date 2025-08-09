#include "SaveMessage.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/message/private/convert/convert.h"
#include "imsdk/src/core/message/opt/db_opt/db_opt.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/common/macro.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <limits>

namespace roc::imsdk::core::message {

/// 保存网络消息
std::vector<std::shared_ptr<model::MessageModel>> SaveMessage::save_net_msgs(W_SDK_ROOT, std::vector<const network::MsgData *> msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});
    
    // 转换为 db 消息
    auto db_msgs = base::util::transform(msgs, [](const network::MsgData *msg) {
        return core::message::Convert::convert_net_msg_to_db_msg(msg);
    });

    // 保存到数据库
    bool ret = message::dbopt::insert_message(w_sdk_root, db_msgs);
    if (!ret) {
        return {};
    }

    // 转换为 sdk 消息
    auto sdk_msgs = base::util::transform(db_msgs, [](const std::shared_ptr<core::message::MessageORM> &msg) {
        return core::message::Convert::convert_db_msg_to_sdk_msg(msg.get());
    });

    // 自动更新消息区间
    update_message_range_for_message(w_sdk_root, sdk_msgs);

    return sdk_msgs;
}

/// 设置 SDK 消息
void SaveMessage::set_sdk_msg(W_SDK_ROOT, const core::message::MessageORM *msg) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    
    CHECK_POINTER_OR_RETURN_VOID(msg);

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    // 转换为SDK消息并保存到缓存
    auto sdk_msg = core::message::Convert::convert_db_msg_to_sdk_msg(msg);
    if (sdk_msg) {
        // 通过友元关系访问MessageManager的私有成员
        msg_manager->msg_cache_[msg->client_msg_id] = sdk_msg;
    }
}

/// 根据 ID 获取 SDK 消息
std::shared_ptr<model::MessageModel> SaveMessage::sdk_msg_for_id(W_SDK_ROOT, const std::string &msg_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, nullptr);

    // 通过友元关系访问MessageManager的私有成员
    auto it = msg_manager->msg_cache_.find(msg_id);
    if (it != msg_manager->msg_cache_.end()) {
        return it->second;
    }
    return nullptr;
}

/// 更新消息区间
void SaveMessage::update_message_range_for_message(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    
    if (sdk_msgs.empty()) {
        return;
    }

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VOID(msg_manager);

    // 按会话ID分组消息
    std::unordered_map<std::string, std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>>> conv_messages;
    for (const auto& msg : sdk_msgs) {
        if (msg) {
            std::string conv_id = msg->conversation_id();
            conv_messages[conv_id].push_back(msg);
        }
    }

    // 为每个会话更新消息区间
    for (const auto& [conv_id, messages] : conv_messages) {
        if (messages.empty()) {
            continue;
        }

        // 找到最小和最大的服务器序号
        int64_t min_seq = std::numeric_limits<int64_t>::max();
        int64_t max_seq = std::numeric_limits<int64_t>::min();
        
        for (const auto& msg : messages) {
            int64_t server_seq = msg->server_order_index();
            if (server_seq > 0) {
                min_seq = std::min(min_seq, server_seq);
                max_seq = std::max(max_seq, server_seq);
            }
        }

        // 如果找到了有效的序号，更新消息区间
        if (min_seq != std::numeric_limits<int64_t>::max() && max_seq != std::numeric_limits<int64_t>::min()) {
            // 这里可以通过MessageManager更新会话的消息区间
            // TODO: 实现具体的区间更新逻辑
        }
    }
}

/// 获取会话的缺失消息区间
std::vector<std::pair<int64_t, int64_t>> SaveMessage::empty_message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, {});

    // 获取会话的现有消息区间
    auto msg_ranges = message::SaveMessage::message_range_for_conv_id(w_sdk_root, conv_id);
    
    // 计算空缺区间
    if (msg_ranges.empty()) {
        return {};
    }

    std::vector<std::pair<int64_t, int64_t>> empty_ranges;
    std::pair<int64_t, int64_t> last_range = {0, 0};

    for (const auto &range : msg_ranges) {
        std::pair<int64_t, int64_t> new_range = {last_range.second + 1, range.first - 1};
        if (new_range.first <= new_range.second) {
            empty_ranges.push_back(new_range);
        }
        last_range = range;
    }

    return empty_ranges;
}

/// 生成客户端消息 ID
std::string SaveMessage::generate_client_msg_id() {
    // 使用时间戳和随机数生成唯一的客户端消息ID
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    std::stringstream ss;
    ss << "client_" 
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
       << "_" << std::setfill('0') << std::setw(3) << ms.count()
       << "_" << dis(gen);
    
    return ss.str();
}

/// 获取会话的消息区间
std::vector<std::pair<int64_t, int64_t>> SaveMessage::message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, {});

    return {};
}


} // namespace roc::imsdk::core::message
