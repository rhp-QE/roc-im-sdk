#include "MessageRange.h"

#include "base/utils/utils.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <MMKV/MMKV.h>
#include <algorithm>
#include <unordered_map>
#include <base/utils/utils.h>

namespace roc::imsdk::service {


std::vector<std::pair<int64_t, int64_t>> convert_to_ranges(std::vector<int64_t> &seqs) {
    if (seqs.empty()) {
        return {};
    }

    std::sort(seqs.begin(), seqs.end());

    std::vector<std::pair<int64_t, int64_t>> ranges;

    int64_t start = seqs[0];
    int64_t end = start;

    for (const auto &seq : seqs) {
        if (seq == end + 1) {
            end = seq;
        } else if (seq > end + 1) {
            ranges.push_back({start, end});

            start = seq;
            end = seq;
        }
    }

    ranges.push_back({start, end});

    return ranges;
}

std::vector<std::pair<int64_t, int64_t>> merge_ranges(std::vector<std::pair<int64_t, int64_t>> &ranges) {
    std::sort(ranges.begin(), ranges.end(), [](const std::pair<int64_t, int64_t> &a, const std::pair<int64_t, int64_t> &b) -> bool {
        return a.first < b.first;
    });

    std::vector<std::pair<int64_t, int64_t>> merged_ranges;
    
    std::pair<int64_t, int64_t> current_range = ranges[0];
    for(const auto &range : ranges) {
        if (range.first <= current_range.second + 1) {
            current_range.second = std::max(current_range.second, range.second);
        } else {
            merged_ranges.push_back(current_range);
            current_range = range;
        }
    }
    merged_ranges.push_back(current_range);

    return merged_ranges;
}

MessageRange::MessageRange(std::weak_ptr<imsdk::SDKRoot> w_sdk_root) : w_sdk_root_(w_sdk_root) {

}

MessageRange::~MessageRange() {

}

void MessageRange::update_messgae_range_for_message(std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs) {

    // 按会话对消息分类
    auto conv_msg_map = base::util::group_by_key(sdk_msgs, [](const std::shared_ptr<roc::imsdk::model::MessageModel> &sdk_msg) -> std::string {
        return sdk_msg->conversation_id();
    });


    // 对每个会话进行消息区间合并
    for (const auto &conv_msg : conv_msg_map) {
        std::vector<int64_t> seqs = base::util::transform(conv_msg.second, [](const std::shared_ptr<roc::imsdk::model::MessageModel> &sdk_msg) -> int64_t {
            return sdk_msg->server_order_index();
        });

        std::vector<std::pair<int64_t, int64_t>> ranges = convert_to_ranges(seqs);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto origin_ranges = conv_msg_range_map_[conv_msg.first];
            for (const auto &range : origin_ranges) {
                ranges.emplace_back(range);
            }
            conv_msg_range_map_[conv_msg.first] = merge_ranges(ranges);

            // 写入到db
        }
    }

}

std::vector<std::pair<int64_t, int64_t>> MessageRange::message_range_for_conv_id(std::string conv_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (conv_msg_range_map_.find(conv_id) == conv_msg_range_map_.end()) {
        return {};
    }
    return conv_msg_range_map_[conv_id];
}

} // namespace roc::imsdk::service