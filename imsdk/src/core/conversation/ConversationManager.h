#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/base/include/containers/ThreadSafeUnorderedMap.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"

// Forward declaration
namespace roc::imsdk::core::conversation {
    class SaveConversation;
}

namespace roc::imsdk::core {

class ConversationManager : public roc::base::uncopyable {
public:
    ConversationManager(std::weak_ptr<SDKRoot> w_sdk_root, boost::asio::io_context::executor_type executor);
    ~ConversationManager();

    // 组件加载完成后的初始化
    void all_component_did_load();

    boost::asio::strand<boost::asio::io_context::executor_type> conv_strand();

    model::OnConvUpdateCallbackType& on_conv_update_callback();

    // =============================  conversation api  ======================================

    /// 会话更新回调
    void on_conv_update(model::OnConvUpdateCallbackType callback);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>>
        conv_for_id(std::string conv_id);

    /// 查询会话列表
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        convs_for_user_id(std::string user_id, int64_t cursor, int64_t limit);

    /// 用户登录时获取首屏会话，后续加载更多会话 调用 convs_for_user_id
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        convs_when_login();

    /// 创建会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        create_conv(std::vector<std::string> member_user_ids, std::string conv_name);

    /// 设置会话置顶
    boost::asio::awaitable<bool>
        set_conv_top(std::string conv_id, bool is_top);

    /// 设置会话免打扰
    boost::asio::awaitable<bool>
        set_conv_mute(std::string conv_id, bool is_mute);

    /// 删除会话
    boost::asio::awaitable<bool>
        delete_conv(std::string conv_id);

    /// =======================================================================================

private:
    std::weak_ptr<SDKRoot> w_sdk_root_;
    std::atomic<int64_t> cursor_ = -1;
    
    /// 会话缓存
    base::containers::ThreadSafeUnorderedMap<std::string, std::shared_ptr<model::ConversationModel>> conv_cache_;
    
    /// 会话更新回调
    model::OnConvUpdateCallbackType on_conv_update_callback_;

    /// 会话操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> conv_strand_;

    // 友元类，允许SaveConversation访问私有成员
    friend class roc::imsdk::core::conversation::SaveConversation;
    friend class roc::imsdk::core::conversation::Convert;
};

} // namespace roc::imsdk::core