#include "SaveMessage.h"

#include "base/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/convert/Convert.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <cstdint>

namespace roc::imsdk::core::message {

/// 保存网络消息
boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
SaveMessage::save_net_messages(W_SDK_ROOT, std::vector<const network::MsgData *> msgs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());
    
    // 转换为 db 消息
    auto db_msgs = base::util::transform(msgs, [w_sdk_root](const network::MsgData *msg) {
        return core::message::Convert::convert_net_msg_to_db_msg(w_sdk_root, msg);
    }); 

    co_return co_await save_db_msgs(w_sdk_root, std::move(db_msgs));
}

boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
SaveMessage::save_db_msgs(W_SDK_ROOT, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());

    auto msg_manager = sdk_root->message_manager();

    // 转换为 sdk 消息
    auto sdk_msgs = base::util::transform(db_msgs, [w_sdk_root](const std::shared_ptr<core::message::MessageORM> &msg) {
        return core::message::Convert::convert_db_msg_to_sdk_msg(w_sdk_root, msg.get());
    });

    /// 在同一个线程内执行 确保 db 和 缓存的一致性
    auto saved_msgs = co_await boost::asio::co_spawn(msg_manager->msg_strand(), [sdk_msgs = std::move(sdk_msgs), db_msgs = std::move(db_msgs), w_sdk_root]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> {
        // 保存到数据库
        bool ret = message::DBOpt::insert_or_replace_message(w_sdk_root, db_msgs);
        if (!ret) {
            co_return std::vector<std::shared_ptr<model::MessageModel>>();
        }

        // 更新缓存
        co_return update_msg_cache(w_sdk_root, std::move(sdk_msgs));
    }, boost::asio::use_awaitable);

    // 自动更新消息区间
    update_message_range_for_message(w_sdk_root, saved_msgs);

    // 更新会话的最大 order_index
    update_msg_order_in_conv(w_sdk_root, saved_msgs);

    co_return saved_msgs;
}

/// 更新会话的最大 order_index
void SaveMessage::update_msg_order_in_conv(W_SDK_ROOT, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    for (const auto &msg : sdk_msgs) {
        message::DBOpt::set_msg_order_in_conv(w_sdk_root, msg->conversation_id(), msg->client_order_index());
    }
}

/// 根据 ID 获取 SDK 消息
boost::asio::awaitable<std::shared_ptr<model::MessageModel>>
SaveMessage::sdk_msg_for_id(W_SDK_ROOT, const std::string &msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->message_manager();

    if (msg_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取
    auto sdk_msg_opt = msg_manager->msg_cache_.at(msg_id);
    if (sdk_msg_opt) {
        co_return sdk_msg_opt.value();
    }

    auto sdk_msg = co_await boost::asio::co_spawn(msg_manager->msg_strand(), [w_sdk_root, msg_id]() -> boost::asio::awaitable<std::shared_ptr<model::MessageModel>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

        // 二次检查
        auto result = sdk_root->message_manager()->msg_cache_.at(msg_id);
        if (result) {
            co_return result.value();
        }

        /// 从DB中获取
        auto sdk_msg_copy = message::DBOpt::message_for_id(w_sdk_root, msg_id);
        if (!sdk_msg_copy) {
            co_return nullptr;
        }
    
        /// 更新缓存
        auto sdk_msgs = update_msg_cache(w_sdk_root, {std::move(sdk_msg_copy)});
        co_return sdk_msgs.size() > 0 ? sdk_msgs.front() : nullptr;

    }, boost::asio::use_awaitable);

    co_return sdk_msg;
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

        /// 如果缓存中没有区间，则从DB中获取
        auto old_ranges = msg_manager->msg_range_cache_.at(conv_id);
        if (!old_ranges) {
            LOG_INFO("MsgManager", "no range in cahce, get from db. 【conv_id】: {}", conv_id);
            input_ranges = merge_ranges(input_ranges, DBOpt::message_range(w_sdk_root, conv_id));
        }

        auto new_ranges = msg_manager->msg_range_cache_.modify_or_create(conv_id, [input_ranges](std::vector<std::pair<int64_t, int64_t>> &current_ranges) {
            current_ranges = merge_ranges(input_ranges, current_ranges);
        });

        message::DBOpt::save_message_range(w_sdk_root, new_ranges, conv_id);
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

boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> 
SaveMessage::load_message_from_db(W_SDK_ROOT, std::string conv_id, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->message_manager();

    auto sdk_msgs = co_await boost::asio::co_spawn(msg_manager->msg_strand(), [=]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());

        /// 从DB 中获取消息
        auto sdk_msgs_copy = message::DBOpt::query_messages_for_conv_id(w_sdk_root, conv_id, cursor, limit, forward);

        /// 更新消息缓存
        co_return update_msg_cache(w_sdk_root, sdk_msgs_copy);

    }, boost::asio::use_awaitable);
    
    auto result = std::make_shared<model::LoadConvMessagesResult>();
    result->cursor = sdk_msgs.empty() ? -1 : sdk_msgs.back()->client_order_index();
    result->has_more = false;
    result->messages = std::move(sdk_msgs);

    co_return result;
}

std::vector<std::pair<int64_t, int64_t>> SaveMessage::load_message_range_from_db(W_SDK_ROOT, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    return message::DBOpt::message_range(sdk_root, conv_id);
}

/// 生成客户端消息 ID
std::string SaveMessage::generate_client_msg_id() {
    return base::util::uuid();
}

/// 设置消息为已读
bool SaveMessage::mark_messages_as_read(W_SDK_ROOT, const std::vector<std::string> &msg_ids) {
    // TODO: 实现具体的消息已读逻辑
    return false;
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
    auto it = msg_manager->msg_range_cache_.at(conv_id);
    if (it) {
        return it.value();
    }

    return {};
}

/// 更新消息缓存
std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> SaveMessage::update_msg_cache(W_SDK_ROOT, std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});
    
    auto msg_manager = sdk_root->message_manager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, {});

    std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> updated_msgs;
    
    for (const auto &sdk_msg : sdk_msgs) {
        if (sdk_msg == nullptr) {
            continue;
        }

        auto cache_sdk_msg = msg_manager->msg_cache_.at(sdk_msg->client_msg_id(), std::make_shared<model::MessageModel>());

        cache_sdk_msg->move_from(std::move(*sdk_msg));
        updated_msgs.push_back(cache_sdk_msg);
    }

    return updated_msgs;
}

} // namespace roc::imsdk::core::message
