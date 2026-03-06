# 子目录 AGENT：`imsdk/src/core/message/private/send`

## 1. 职责定位

- 负责**单条消息发送链路**的完整编排：  
  - 参数校验、`client_msg_id` 生成。  
  - 构造协议模型 `sdkws::MessageData` / `BatchSendMessageRequest`。  
  - 调用 `MessageDataSource`/`DBOpt` 完成本地落库与顺序索引。  
  - 通过 `ConnectionManager` 封装 `FrontierMessage` 并发起网络请求。  
  - 解析 `BatchSendMessageResponse`，用服务端返回结果覆盖本地记录并回调业务。  

## 2. 关键类

- **`SendMessageController`**  
  - `SendMessage`：公共入口，负责同步路径（校验 + 落库 + 返回「发送中」消息），异步路径通过 `co_spawn` 投递到 `net_io_context`。  
  - 私有前缀 `p_` 的方法只在本目录内使用，用于拆分复杂逻辑（如 `p_checkSendContext`, `p_generateClientMsgId`, `p_convertSendContextToSdkwsMessage`, `p_convertSendContextToMessageOrm`, `p_asyncSendMessage`, `p_request` 等）。  

## 3. 线程与依赖约束

- `SendMessage` 在调用协程上下文中运行同步部分；网络发送与结果处理在 `sdk_root->net_io_context()` 上执行。  
- 只能通过 `MessageManager` 获取 `MessageDataSource`/`DBOpt` 等依赖，不得自行持有全局单例。  

## 4. 局部编写规范

- 新增发送相关能力（如「带附件发送」、「特殊消息类型」）时：  
  - 优先在本目录扩展转换和编排逻辑；  
  - 不要在 `MessageManager` 里堆积过多分支。  
- 所有与协议结构变更相关的修改，必须同步更新：  
  - 本文件（简要说明新增职责）；  
  - `private/convert/AGENT.md`（如果涉及模型转换）；  
  - 相关时序/链路文档。  

