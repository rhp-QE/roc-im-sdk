#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"

#include <boost/asio/awaitable.hpp>
#include <memory>
#include <string>
#include <vector>

namespace roc::imsdk::core::conversation {

class CreateConversation {
public:
    /// 创建会话
    static boost::asio::awaitable<std::shared_ptr<model::ConversationModel>>
        CreateConv(CONTEXT_T, std::vector<std::string> member_user_ids, std::string conv_name);
};

} // namespace roc::imsdk::core::conversation


