#pragma once

#include "imsdk/src/core/common/macro.h"
#include "imsdk/src/core/sdkroot/SDKRoot.h"
#include "imsdk/src/core/network/proto/sdkws.pb.h"
#include "imsdk/base/include/network/Error.h"

#include <boost/asio/awaitable.hpp>
#include <memory>
#include <expected>

namespace roc::imsdk::core::message {

class ConvMessagesFetcher {
public:
    explicit ConvMessagesFetcher(std::weak_ptr<SDKRoot> sdk_root);

    /// 获取会话消息列表
    boost::asio::awaitable<void> FetchConvMessageList(CTX_T, std::string conv_id);
    
    /// 获取指定区间的会话消息
    asio::awaitable<void> FetchConvMessageListForRange(CTX_T, std::string conv_id, std::pair<int64_t, int64_t> range);

private:
    /// 生成请求
    std::unique_ptr<network::FetchConvMessageListRequest> p_MakeFetchConvMessageListReq(CTX_T, std::string conv_id, std::pair<int64_t, int64_t> range);
    
    /// 处理返回数据
    void p_HandleFetchConvMessgaeListResp(CTX_T, std::unique_ptr<network::FetchConvMessageListResponse> resp);
    
    /// 发送获取会话消息列表请求
    boost::asio::awaitable<std::expected<std::unique_ptr<network::FetchConvMessageListResponse>, roc::error::Error>> 
        p_request(CTX_T, network::FetchConvMessageListRequest *request);
    
    std::weak_ptr<SDKRoot> w_sdk_root;
};

} // namespace roc::imsdk::core::message