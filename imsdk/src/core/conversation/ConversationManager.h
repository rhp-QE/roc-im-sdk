#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <expected>
#include "imsdk/src/include/IMSDK.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/src/core/conversation/db_model/ConversationORM.h"
#include "imsdk/base/include/network/Error.h"


// Forward declaration
namespace roc::imsdk::core::conversation {
    class UserMessageFetcher;
    class ConvDatasource;
    class ReceiveConversation;
    class DBOpt;
    class Convert;
    class ConversationStatusHandler;
}

namespace roc::imsdk::core {

class ConversationManager : public roc::base::uncopyable {
public:
    ConversationManager(std::shared_ptr<SDKRoot> sdk_root);
    ~ConversationManager();

    // 组件加载完成后的初始化
    void AllComponentDidLoad();

    model::OnConversationsCallbackTy& OnConversationsCallback();

    // =============================  conversation api  ======================================

    /// 会话更新回调
    void OnConvUpdate(model::OnConversationsCallbackTy callback);

    /// 查询会话
    boost::asio::awaitable<std::shared_ptr<model::ConversationModel>>
        ConvForId(std::string conv_id);

    /// 查询会话列表
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        ConvsForUserId(std::string user_id, int64_t cursor, int64_t limit);

    /// 用户登录时获取首屏会话，后续加载更多会话 调用 convs_for_user_id
    boost::asio::awaitable<std::shared_ptr<model::LoadUserConvsResult>>
        ConvsWhenLogin();

    /// 创建群聊
    boost::asio::awaitable<std::expected<std::shared_ptr<model::ConversationModel>, roc::error::Error>>
        CreateGroup(const model::CreateGroupContext &context);

    /// 邀请群成员
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        InviteGroupMembers(const model::InviteGroupMembersContext &context);

    /// 设置会话置顶
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetConvTop(std::string conv_id, bool is_top);

    /// 设置会话免打扰
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetConvMute(std::string conv_id, bool is_mute);

    /// 设置会话拉黑
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetConvBlock(std::string conv_id, bool is_block);

    /// 设置会话同步扩展字段
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetConvSyncExt(std::string conv_id, std::string key, std::string value);

    /// 设置会话本地扩展字段（仅本地，不发送网络请求）
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        SetConvLocalExt(std::string conv_id, std::string key, std::string value);

    /// 设置会话已读
    boost::asio::awaitable<bool>
        SetConvRead(std::string conv_id);

    /// 删除会话
    boost::asio::awaitable<std::expected<bool, roc::error::Error>>
        DeleteConv(std::string conv_id);

    /// =======================================================================================

private:
    /// 初始化子组件
    void p_InitSubComponents();

    std::weak_ptr<SDKRoot> w_sdk_root;
    std::atomic<int64_t> cursor_ = -1;
    
    /// 会话更新回调
    model::OnConversationsCallbackTy on_convs_callback_;

    // 友元类，允许子组件访问私有成员
    friend class roc::imsdk::core::conversation::DBOpt;
    friend class roc::imsdk::core::conversation::Convert;
    friend class roc::imsdk::core::conversation::ConvDatasource;
    friend class roc::imsdk::core::conversation::UserMessageFetcher;
    friend class roc::imsdk::core::conversation::ReceiveConversation;
    friend class roc::imsdk::core::conversation::ConversationStatusHandler;

    /// 子组件
    std::unique_ptr<conversation::DBOpt> db_opt;
    std::unique_ptr<conversation::Convert> convert;
    std::unique_ptr<conversation::ConvDatasource> conv_datasource;
    std::unique_ptr<conversation::UserMessageFetcher> user_message_fetcher;
    std::unique_ptr<conversation::ReceiveConversation> receive_conversation;
    std::unique_ptr<conversation::ConversationStatusHandler> conversation_status_handler;
};

} // namespace roc::imsdk::core