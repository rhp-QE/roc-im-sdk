#pragma once

#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include <unordered_map>

// Forward declaration
namespace roc::imsdk::core::conversation {
    class SaveConversation;
}

namespace roc::imsdk::core {

class ConversationManager : public roc::base::uncopyable {
public:
    ConversationManager(std::weak_ptr<SDKRoot> w_sdk_root);
    ~ConversationManager();

    // 组件加载完成后的初始化
    void all_component_did_load();

    std::shared_ptr<model::ConversationModel> sdk_conv_for_id(std::string conv_id);

    /// 游标
    int64_t cursor();
    void set_cursor(int64_t cursor);

    /// 保存网络会话
    std::vector<std::shared_ptr<model::ConversationModel>> save_net_convs(std::vector<std::shared_ptr<network::ConversationInfo>> convs);

    // =============================  conversation api  ======================================

    /// 会话更新回调
    void on_conv_update(model::OnConvUpdateCallbackType callback);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> conv_for_id(std::string conv_id);

    /// 查询会话列表
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit);

    /// 用户登录时获取首屏会话，后续加载更多会话 调用 convs_for_user_id
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>> convs_when_login();

    /// 设置会话置顶
    boost::asio::awaitable<bool> set_conv_top(std::string conv_id, bool is_top);

    /// 设置会话免打扰
    boost::asio::awaitable<bool> set_conv_mute(std::string conv_id, bool is_mute);

    /// 删除会话
    boost::asio::awaitable<bool> delete_conv(std::string conv_id);

    /// =======================================================================================

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
    int64_t cursor_ = -1;
    
    /// 会话缓存
    std::unordered_map<std::string, std::shared_ptr<model::ConversationModel>> conv_cache_;
    
    /// 会话更新回调
    model::OnConvUpdateCallbackType on_conv_update_callback_;

    // 友元类，允许SaveConversation访问私有成员
    friend class roc::imsdk::core::conversation::SaveConversation;
    friend class roc::imsdk::core::conversation::Convert;
};

} // namespace roc::imsdk::core