#include "MessageDataSource.h"

#include "imsdk/base/include/utils/utils.h"
#include "imsdk/src/core/common/logger_macro.h"
#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/message/MessageManager.h"
#include "imsdk/src/core/message/private/db_opt/DBOpt.h"
#include "imsdk/src/core/message/private/convert/Convert.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <cstdint>
#include <string>

namespace roc::imsdk::core::message {

MessageDataSource::MessageDataSource(std::weak_ptr<SDKRoot> sdk_root) 
    : w_sdk_root(sdk_root) {
}

// 私有方法 =======================

/// 保存消息日志
void MessageDataSource::p_LogSaveMessages(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    std::string info = "[";
    for (auto ptr : db_msgs) {
        info += ", message_id = " + ptr->client_msg_id + " conv_id = " + ptr->conversation_id + " order = " + std::to_string(ptr->client_order_index);
    }
    info += "]";
    
    LOG_INFO("message", "call save message, {}", info);
}
// ===============================



/// 保存网络消息
boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
MessageDataSource::SaveNetMessages(CTX_T, std::vector<const network::MsgData *> msgs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());
    
    auto msg_manager = sdk_root->MessageManager();
    // 转换为 db 消息
    auto db_msgs = base::util::transform(msgs, [msg_manager, w_sdk_root = w_sdk_root, call_track_id](const network::MsgData *msg) {
        return msg_manager->convert->ConvertNetMsgToDbMsg(CTX_V, msg);
    }); 

    co_return co_await SaveDbMsgs(CTX_V, std::move(db_msgs));
}

boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> 
MessageDataSource::SaveDbMsgs(CTX_T, std::vector<std::shared_ptr<core::message::MessageORM>> db_msgs) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());

    p_LogSaveMessages(CTX_V, db_msgs);

    auto msg_manager = sdk_root->MessageManager();

    // 转换为 sdk 消息
    auto sdk_msgs = base::util::transform(db_msgs, [msg_manager, w_sdk_root = w_sdk_root, call_track_id](const std::shared_ptr<core::message::MessageORM> &msg) {
        return msg_manager->convert->ConvertDbMsgToSdkMsgTmp(CTX_V, msg.get());
    });

    /// 在同一个线程内执行 确保 db 和 缓存的一致性
    auto saved_msgs = co_await boost::asio::co_spawn(msg_manager->MsgStrand(), [this, msg_manager, sdk_msgs = std::move(sdk_msgs), db_msgs = std::move(db_msgs), w_sdk_root = w_sdk_root, call_track_id]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> {
        // 保存到数据库
        bool ret = msg_manager->db_opt->InsertOrReplaceMessage(CTX_V, db_msgs);

        // // 排除 local_ext 和 client_order_index 字段（黑名单模式）
        // WCDB::Fields not_update_when_exit({
        //     WCDB_FIELD(core::message::MessageORM::local_ext),
        //     WCDB_FIELD(core::message::MessageORM::client_order_index)
        // });
        // bool ret = message::DBOpt::insert_or_update_message(CTX_V, db_msgs, not_update_when_exit, 1);
        if (!ret) {
            co_return std::vector<std::shared_ptr<model::MessageModel>>();
        }

        // 更新缓存
        co_return UpdateMsgCache(CTX_V, std::move(sdk_msgs));
    }, boost::asio::use_awaitable);

    // 自动更新消息区间
    p_UpdateMessageRangeForMessage(CTX_V, saved_msgs);

    // 更新会话的最大 order_index
    UpdateMsgOrderInConv(CTX_V, saved_msgs);

    co_return saved_msgs;
}

/// 更新会话的最大 order_index
void MessageDataSource::UpdateMsgOrderInConv(CTX_T, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);

    auto msg_manager = sdk_root->MessageManager();
    for (const auto &msg : sdk_msgs) {
        msg_manager->db_opt->SetMsgOrderInConv(CTX_V, msg->conversation_id(), msg->client_order_index());
    }
}

/// 根据 ID 获取 SDK 消息
boost::asio::awaitable<std::shared_ptr<model::MessageModel>>
MessageDataSource::SdkMsgForId(CTX_T, const std::string &msg_id) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->MessageManager();

    if (msg_id.empty()) {
        co_return nullptr;
    }

    /// 从缓存中获取
    auto sdk_msg_opt = message_cache_.at(msg_id);
    if (sdk_msg_opt) {
        co_return sdk_msg_opt.value();
    }

    auto sdk_msg = co_await boost::asio::co_spawn(msg_manager->MsgStrand(), [=, w_sdk_root = w_sdk_root]() -> boost::asio::awaitable<std::shared_ptr<model::MessageModel>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);
        auto msg_manager = sdk_root->MessageManager();

        // 二次检查
        auto result = sdk_root->MessageManager()->message_data_source->message_cache_.at(msg_id);
        if (result) {
            co_return result.value();
        }

        /// 从DB中获取
        auto sdk_msg_copy = msg_manager->db_opt->MessageForId(CTX_V, msg_id);
        if (!sdk_msg_copy) {
            co_return nullptr;
        }

        /// 更新缓存
        auto sdk_msgs = msg_manager->message_data_source->UpdateMsgCache(CTX_V, {std::move(sdk_msg_copy)});
        co_return sdk_msgs.size() > 0 ? sdk_msgs.front() : nullptr;

    }, boost::asio::use_awaitable);

    co_return sdk_msg;
}

/// 更新消息区间
void MessageDataSource::p_UpdateMessageRangeForMessage(CTX_T, const std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> &sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VOID(w_sdk_root);
    
    if (sdk_msgs.empty()) {
        return;
    }

    auto msg_manager = sdk_root->MessageManager();
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

        auto input_ranges = p_GenerateRange(seqs);

        /// 如果缓存中没有区间，则从DB中获取
        auto old_ranges = message_range_cache_.at(conv_id);
        if (!old_ranges) {
            LOG_INFO("MsgManager", "no range in cahce, get from db. conv_id: {}", conv_id);
            input_ranges = p_MergeRanges(input_ranges, msg_manager->db_opt->MessageRange(CTX_V, conv_id));
        }

        /// 更新缓存
        auto new_ranges = message_range_cache_.modify_or_create(conv_id, [input_ranges](std::vector<std::pair<int64_t, int64_t>> &current_ranges) {
            current_ranges = p_MergeRanges(input_ranges, current_ranges);
        });

        /// 保存到数据库
        msg_manager->db_opt->SaveMessageRange(CTX_V, new_ranges, conv_id);
    }
}

/// 获取会话的缺失消息区间
std::vector<std::pair<int64_t, int64_t>> MessageDataSource::EmptyMessageRangeForConvId(CTX_T, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    // 获取会话的现有消息区间
    auto msg_ranges = p_MessageRangeForConvId(CTX_V, conv_id);
    
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
MessageDataSource::LoadMessageFromDb(CTX_T, std::string conv_id, int64_t cursor, int64_t limit, bool forward) {
    CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, nullptr);

    auto msg_manager = sdk_root->MessageManager();

    auto sdk_msgs = co_await boost::asio::co_spawn(msg_manager->MsgStrand(), [this, msg_manager, w_sdk_root = w_sdk_root, call_track_id, conv_id, cursor, limit, forward]() -> boost::asio::awaitable<std::vector<std::shared_ptr<model::MessageModel>>> {
        CHECK_ROOT_OR_CO_RETURN_VALUE(w_sdk_root, std::vector<std::shared_ptr<model::MessageModel>>());

        /// 从DB 中获取消息
        auto sdk_msgs_copy = msg_manager->db_opt->QueryMessagesForConvId(CTX_V, conv_id, cursor, limit, forward);

        /// 更新消息缓存
        co_return UpdateMsgCache(CTX_V, sdk_msgs_copy);

    }, boost::asio::use_awaitable);
    
    auto result = std::make_shared<model::LoadConvMessagesResult>();
    result->cursor = sdk_msgs.empty() ? -1 : sdk_msgs.back()->client_order_index();
    result->has_more = false;
    result->messages = std::move(sdk_msgs);

    co_return result;
}

/// 设置消息为已读
bool MessageDataSource::MarkMessagesAsRead(CTX_T, const std::vector<std::string> &msg_ids) {
    // TODO: 实现具体的消息已读逻辑
    return false;
}

/// 给定一个数字序列，生成若干区间。一个区间内的所有数字都在给定的数组序列内。区间内数字是连续的，左右都闭合。
std::vector<std::pair<int64_t, int64_t>> MessageDataSource::p_GenerateRange(std::vector<int64_t> seqs) {
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
std::vector<std::pair<int64_t, int64_t>> MessageDataSource::p_MergeRanges(std::vector<std::pair<int64_t, int64_t>> first, std::vector<std::pair<int64_t, int64_t>> second) {
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
std::vector<std::pair<int64_t, int64_t>> MessageDataSource::p_MessageRangeForConvId(CTX_T, const std::string &conv_id) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});

    auto it = message_range_cache_.at(conv_id);
    if (it) {
        return it.value();
    } else { // 从数据库中兜底获取
        LOG_INFO("MessageDataSource", "no message range in cache, need load from db, cid = {}", conv_id)
        auto msg_ranges = sdk_root->MessageManager()->db_opt->MessageRange(CTX_V, conv_id);
        message_range_cache_.insert_or_assign(conv_id, msg_ranges);
        return msg_ranges;
    }
}

/// 更新消息缓存
std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> MessageDataSource::UpdateMsgCache(CTX_T, std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs) {
    CHECK_ROOT_OR_RETURN_VALUE(w_sdk_root, {});
    
    auto msg_manager = sdk_root->MessageManager();
    CHECK_POINTER_OR_RETURN_VALUE(msg_manager, {});

    std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> updated_msgs;
    
    for (const auto &sdk_msg : sdk_msgs) {
        if (sdk_msg == nullptr) {
            continue;
        }

        auto cache_sdk_msg = message_cache_.at(sdk_msg->client_msg_id(), std::make_shared<model::MessageModel>());

        cache_sdk_msg->move_from(std::move(*sdk_msg));
        updated_msgs.push_back(cache_sdk_msg);
    }

    return updated_msgs;
}

} // namespace roc::imsdk::core::message
