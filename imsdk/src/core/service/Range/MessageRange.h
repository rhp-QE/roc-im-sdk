#pragma once

#include <memory>
#include <vector>
#include <string>

#include "base/Uncopyable.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"

class MMKV;
namespace roc::imsdk::model {
class MessageModel;
}

namespace roc::imsdk::service {

class MessageRange : public roc::base::uncopyable {
public:
    MessageRange(std::weak_ptr<imsdk::SDKRoot> w_sdk_root);
    ~MessageRange();

/// 更新消息区间
void update_messgae_range_for_message(std::vector<std::shared_ptr<roc::imsdk::model::MessageModel>> sdk_msgs);

/// 获取区间
std::vector<std::pair<int64_t, int64_t>> message_range_for_conv_id(std::string conv_id);

private:
    std::weak_ptr<imsdk::SDKRoot> w_sdk_root_;
    std::unordered_map<std::string, std::vector<std::pair<int64_t, int64_t>>> conv_msg_range_map_;
    std::mutex mutex_; // 保护 conv_msg_range_map_

};

}