#include "imsdk/src/core/service/ServiceEntrace.h"
#include "imsdk/src/core/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/service/Fetcher/UserMessageFetcher.h"
#include <boost/asio/awaitable.hpp>
#include <memory>
#include "base/utils/utils.h"



// 1、用户主动调用接口拉取db 会话
// 2、用户同时监听用户会话更新

// 3、sdk 启动混链拉取 将拉取到的会话回调给 业务层
// 4、sdk 将push 的消息回调给业务， 同时回调会话order 更新

// 5、sdk 将 命令消息中 更新会话 和 更新消息 回调给业务层

namespace roc::imsdk::service {

ServiceEntrace::ServiceEntrace(std::weak_ptr<SDKRoot> sdk_root) : w_sdk_root_(sdk_root) {
}

ServiceEntrace::~ServiceEntrace() = default;

// 在此之前 数据已初始化完成
// 此处进行逻辑 处理
boost::asio::awaitable<void> ServiceEntrace::start_service() {
    CHECK_ROOT_OR_CO_RETURN_VOID(w_sdk_root_)

    // 加载db 消息和会话返回给用户



    // 拉取消息
    bool has_more = true;
    do {
        FetchUserMessageResult result = co_await sdk_root->user_message_fetcher()->fetch_user_messages();

        has_more = result.has_more;

        /// 上抛收到的消息
        base::util::safe_invoke_block(sdk_root->injection()->on_new_message_callback, result.conv_messages_union_vec);

    } while (has_more);


}

}