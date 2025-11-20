#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/base/include/containers/ThreadSafeUnorderedMap.h"


// Forward declaration
namespace roc::imsdk::core::conversation {
    class UserMessageFetcher;
    class SaveConversation;
    class ReceiveConversation;
    class CreateConversation;
    class DBOpt;
    class Convert;
}

namespace roc::imsdk::core {

class ConversationManager : public roc::base::uncopyable {
public:
    ConversationManager(std::shared_ptr<SDKRoot> sdk_root, boost::asio::io_context::executor_type executor);
    ~ConversationManager();

    // 组件加载完成后的初始化
    void AllComponentDidLoad();

    boost::asio::strand<boost::asio::io_context::executor_type> ConvStrand();

    model::OnConvUpdateCallbackType& OnConvUpdateCallback();

    // =============================  conversation api  ======================================

    /// 会话更新回调
    void OnConvUpdate(model::OnConvUpdateCallbackType callback);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>>
        ConvForId(std::string conv_id);

    /// 查询会话列表
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        ConvsForUserId(std::string user_id, int64_t cursor, int64_t limit);

    /// 用户登录时获取首屏会话，后续加载更多会话 调用 convs_for_user_id
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        ConvsWhenLogin();

    /// 创建会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>> 
        CreateConv(std::vector<std::string> member_user_ids, std::string conv_name);

    /// 设置会话置顶
    boost::asio::awaitable<bool>
        SetConvTop(std::string conv_id, bool is_top);

    /// 设置会话免打扰
    boost::asio::awaitable<bool>
        SetConvMute(std::string conv_id, bool is_mute);

    /// 设置会话已读
    boost::asio::awaitable<bool>
        SetConvRead(std::string conv_id);

    /// 删除会话
    boost::asio::awaitable<bool>
        DeleteConv(std::string conv_id);

    /// =======================================================================================

private:
    /// 初始化子组件
    void p_InitSubComponents();

    std::weak_ptr<SDKRoot> w_sdk_root;
    std::atomic<int64_t> cursor_ = -1;
    
    /// 会话缓存
    base::containers::ThreadSafeUnorderedMap<std::string, std::shared_ptr<model::ConversationModel>> conv_cache_;
    
    /// 会话更新回调
    model::OnConvUpdateCallbackType on_conv_update_callback_;

    /// 会话操作串行队列
    boost::asio::strand<boost::asio::io_context::executor_type> conv_strand_;

    // 友元类，允许子组件访问私有成员
    friend class roc::imsdk::core::conversation::Convert;
    friend class roc::imsdk::core::conversation::SaveConversation;
    friend class roc::imsdk::core::conversation::UserMessageFetcher;
    friend class roc::imsdk::core::conversation::DBOpt;
    friend class roc::imsdk::core::conversation::ReceiveConversation;
    friend class roc::imsdk::core::conversation::CreateConversation;

    /// 子组件
    std::unique_ptr<conversation::UserMessageFetcher> user_message_fetcher;
    std::unique_ptr<conversation::SaveConversation> save_conversation;
    std::unique_ptr<conversation::ReceiveConversation> receive_conversation;
    std::unique_ptr<conversation::CreateConversation> create_conversation;
    std::unique_ptr<conversation::DBOpt> db_opt;
    std::unique_ptr<conversation::Convert> convert;
};

} // namespace roc::imsdk::core