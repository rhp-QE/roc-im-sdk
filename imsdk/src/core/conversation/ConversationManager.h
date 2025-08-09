#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

namespace roc::imsdk::core {

class ConversationManager : public roc::base::uncopyable {
public:
    ConversationManager();
    ~ConversationManager();

    void set_sdk_conv(const core::conversation::ConversationORM *conv);
    std::shared_ptr<model::ConversationModel> sdk_conv_for_id(std::string conv_id);

    /// 游标
    int64_t cursor();
    void set_cursor(int64_t cursor);

    /// 保存网络会话
    std::vector<std::shared_ptr<model::ConversationModel>> save_net_convs(std::vector<const network::ConversationInfo *> convs);

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
};

} // namespace roc::imsdk::core