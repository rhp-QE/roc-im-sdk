///
/// @file   MessageCacheLogic.h
/// @brief  消息缓存逻辑
/// @author  RuanHuipeng
/// @date    2025-07-23
/// @version 1.0
///

#ifndef __IMSDK_MESSAGE_SERVICE_CORE_CACHE_LOGIC_H__
#define __IMSDK_MESSAGE_SERVICE_CORE_CACHE_LOGIC_H__

#include "base/Uncopyable.h"
#include "imsdk/src/include/model/message/MessageModel.h"

#include <unordered_map>

namespace roc::imsdk::service {

class MessageCacheLogic : public roc::base::uncopyable {
public:
    MessageCacheLogic();
    ~MessageCacheLogic();

    void add_message(std::shared_ptr<imsdk::model::MessageModel> message);
    void remove_message(std::shared_ptr<imsdk::model::MessageModel> message);
    void update_message(std::shared_ptr<imsdk::model::MessageModel> message);
    std::shared_ptr<imsdk::model::MessageModel> find_message(const std::string &message_id);

private:
    std::unordered_map<std::string, std::shared_ptr<imsdk::model::MessageModel>> message_cache_;
};

} // namespace roc::imsdk

#endif