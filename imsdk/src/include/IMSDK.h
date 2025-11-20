#ifndef ROC_IMSDK_INCLUDE_IMSDK_H
#define ROC_IMSDK_INCLUDE_IMSDK_H

#include <cstdint>
#include <memory>
#include <boost/asio/awaitable.hpp>
#include <netdb.h>

#include "imsdk/src/include/config.h"
#include "imsdk/base/include/uncopyable.h"
#include "imsdk/src/include/injection/log/ILogger.h"
#include "imsdk/src/include/model/message/MessageModel.h"
#include "imsdk/src/include/model/conversation/ConversationModel.h"
#include "imsdk/src/include/model/network.h"


/// 进入消息 Tab
// 1、用户主动调用接口拉取db 会话
// 2、用户同时监听用户会话更新

// 3、sdk 启动混链拉取 将拉取到的会话回调给 业务层
// 4、sdk 将push 的消息回调给业务， 同时回调会话order 更新

// 5、sdk 将 命令消息中 更新会话 和 更新消息 回调给业务层


/// 进入会话
// 1、用户主动调用接口拉取db 消息
// 2、用户同时监听用户消息更新

// 3、sdk 启动单链拉取 进行空洞补齐 将拉取到的消息回调给 业务层
// 4、sdk 将push 的消息回调给业务， 同时回调会话order更新

// 5、sdk 将 命令消息中 更新消息 和 更新会话 回调给业务层

namespace roc::imsdk {

class SDKRoot;

class IMSDK : public std::enable_shared_from_this<IMSDK>,
              public roc::base::uncopyable
{

public:
    IMSDK();
    ~IMSDK();

    // 初始化SDK
    boost::asio::awaitable<bool> InitSdk(const Config config);

    // 运行SDK
    boost::asio::awaitable<bool> run();

    boost::asio::awaitable<bool> LoginOut();


    /// ========================== injection api ==========================

    void InjectLogger(std::shared_ptr<ILogger> logger);
    
    /// ========================== injection api ==========================



    /// =============================  message api  ======================================

    /// 发送消息
    boost::asio::awaitable<std::shared_ptr<model::SendMessageResponse>> 
        SendMessage(const model::SendMsgContext &context, std::function<void(std::shared_ptr<model::SendMessageResponse>)> callback);
    
    /// 消息更新回调
    void OnMessagee(model::OnMessagesCallbackType callback);

    /// 删除消息
    boost::asio::awaitable<bool>
        DeleteMessage(const std::vector<std::string> &msg_ids);

    /// 撤回消息
    boost::asio::awaitable<bool>
        RecallMessage(std::string msg_id);

    /// 修改消息 sync_ext
    boost::asio::awaitable<bool>
        UpdateMessageSyncExt(std::string msg_id, std::string key, std::string value);

    /// 设置消息为已读
    boost::asio::awaitable<bool>
        MarkMessagesAsRead(const std::vector<std::string> &msg_ids);

    /// 查询消息
    boost::asio::awaitable<std::shared_ptr<model::MessageModel>> 
        MessageForId(std::string msg_id);

    /// 查询会话消息
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> 
        MessagesForConvId(std::string conv_id, int64_t cursor, int64_t limit);

    /// 当进入会话时，获取首屏消息。 后续加载更多消息时使用 messages_for_conv
    boost::asio::awaitable<std::shared_ptr<model::LoadConvMessagesResult>> 
        MessagesWhenEnterChat(std::string conv_id);

    /// ==================================================================================



    /// =============================  conversation api  ======================================

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



    // =============================  net api  ==============================================
    
    /// 获取网络状态
    roc::imsdk::network::NetworkStatus
        GetNetworkStatus();

    /// 设置网络状态回调
    void OnNetworkStatusChange(std::function<void(roc::imsdk::network::NetworkStatus)> callback);

    // ======================================================================================

private:
    std::shared_ptr<SDKRoot> sdk_root_;
};

} // namespace roc::imsdk

#endif // ROC_IMSDK_INCLUDE_IMSDK_H