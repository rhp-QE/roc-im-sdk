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

        auto seqs = base::util::transform(messages, [](const std::shared_ptr<roc::imsdk::model::MessageModel> &msg) {
            return msg->server_order_index();
        });

        auto input_ranges = generate_range(seqs);

        auto current_ranges = msg_manager->msg_range_cache_[conv_id];

        msg_manager->msg_range_cache_[conv_id] = merge_ranges(input_ranges, current_ranges);
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
    return base::util::uuid();
}

/// 给定一个数字序列，生成若干区间。一个区间内的所有数字都在给定的数组序列内。区间内数字是连续的，左右都闭合。
std::vector<std::pair<int64_t, int64_t>> SaveMessage::generate_range(std::vector<int64_t> seqs) {
    if (seqs.empty()) {
        return {};
    }
    
    // 排序序列
    std::sort(seqs.begin(), seqs.end());
    
    std::vector<std::pair<int64_t, int64_t>> ranges;
    int64_t start = seqs[0];
    int64_t end = seqs[0];
    
    for (const auto seq : seqs) {
        if (seq == end + 1) {
            // 连续数字，扩展区间
            end = seq;
        } else if (seq > end + 1) {
            // 不连续，保存当前区间并开始新区间
            ranges.emplace_back(start, end);
            start = seq;
            end = seq;
        }
    }
    
    // 添加最后一个区间
    ranges.emplace_back(start, end);
    
    return ranges;
}

/// 给定两个区间数组，合并两个数组，返回一个新的区间数组。合并后的区间数组内的区间是连续的，左右都闭合。
std::vector<std::pair<int64_t, int64_t>> SaveMessage::merge_ranges(std::vector<std::pair<int64_t, int64_t>> first, std::vector<std::pair<int64_t, int64_t>> second) {
    if (first.empty()) {
        return second;
    }
    if (second.empty()) {
        return first;
    }
    
    // 合并两个数组
    std::vector<std::pair<int64_t, int64_t>> merged;
    merged.reserve(first.size() + second.size());
    merged.insert(merged.end(), first.begin(), first.end());
    merged.insert(merged.end(), second.begin(), second.end());
    
    // 按区间起始位置排序
    std::sort(merged.begin(), merged.end());
    
    // 合并重叠或相邻的区间
    std::vector<std::pair<int64_t, int64_t>> result;
    if (!merged.empty()) {
        result.push_back(merged[0]);
        
        for (size_t i = 1; i < merged.size(); ++i) {
            auto& current = merged[i];
            auto& last = result.back();
            
            // 检查是否可以合并：当前区间与上一个区间重叠或相邻
            if (current.first <= last.second + 1) {
                // 可以合并，更新上一个区间的结束位置
                last.second = std::max(last.second, current.second);
            } else {
                // 不能合并，添加新区间
                result.push_back(current);
            }
        }
    }
    
    return result;
}

/// 获取会话的消息区间
std::vector<std::pair<int64_t, int64_t>> SaveMessage::message_range_for_conv_id(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, {});

    // 从消息管理器的缓存中获取会话的消息区间
    auto it = msg_manager->msg_range_cache_.find(conv_id);
    if (it != msg_manager->msg_range_cache_.end()) {
        return it->second;
    }
    
    return {};
}

} // namespace roc::imsdk::core::message
