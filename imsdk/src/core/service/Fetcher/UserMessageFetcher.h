///
/// @file   UserMessageFetcher.h
/// @brief  用户消息获取器
/// @author  RuanHuipeng
/// @date    2025-07-22
/// @version 1.0
///


#pragma once

#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/core/injection/Injection.h"

#include <boost/asio/awaitable.hpp>
#include <memory>

namespace roc::imsdk::service {

struct FetchUserMessageResult {
    bool has_more = false;
    std::vector<injection::ConvMessagesUnion> conv_messages_union_vec;
};


/// 职责
/// 1、拉取混链
/// 2、更新db
/// 3、更新sdk 缓存
/// 4、上抛给业务层
class UserMessageFetcher : public roc::base::uncopyable {
public:
    UserMessageFetcher(std::weak_ptr<SDKRoot> sdk_root);
    ~UserMessageFetcher();

    boost::asio::awaitable<FetchUserMessageResult> 
    fetch_user_messages();

private:
    std::weak_ptr<SDKRoot> sdk_root_;
};

} // namespace roc::imsdk::service