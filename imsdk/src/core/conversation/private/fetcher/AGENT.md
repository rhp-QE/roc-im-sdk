# 子目录 AGENT：`imsdk/src/core/conversation/private/fetcher`

## 1. 职责定位

- 负责用户登录/重进时的**混链会话与消息拉取**，以及本地会话游标的维护与完整性校验：  
  - 构造 `FetchUserRecentConvListRequest` 请求并通过 `ConnectionManager` 下发。  
  - 处理 `FetchUserRecentConvListResponse`，解析出 `ConversationData` 与内嵌 `MessageData`。  
  - 维护本地会话游标（`ChatsCursor`），按区间分批拉取并在必要时发起完整性校验请求。  
  - 将拉取到的消息交给 `MessageManager`，将会话交给 `ReceiveConversation` 做后续落库与上抛。  

## 2. 关键类

- **`UserMessageFetcher`**  
  - `FetchUserMessages`：混链拉取主流程入口，负责分页循环、游标更新与完整性二次校验。  
  - `p_makeFetchUserMessageListReq`：构造 `FetchUserRecentConvListRequest`，填充用户 ID、游标与区间参数。  
  - `p_request`：封装请求为 `network::FrontierMessage`，调用 `ConnectionManager::SendRequest` 并解析响应。  
  - `p_handleFetchedUserMessage`：将响应中的 `ConversationData` 拆出 `MessageData` 与会话列表，分别交由 `MessageManager` 和 `ReceiveConversation` 处理。  
  - `p_doubleCheckUserMessageIntegrity`：在拉取结束后下发完整性校验请求，根据结果触发补齐并更新会话游标。  

## 3. 线程与依赖约束

- `FetchUserMessages` 在 `SDKRoot::net_io_context()` 上运行网络请求与协议解析逻辑；处理完成后通过 `sdk_io_context` 回到消息/会话领域，必须遵守宪章中的线程模型约束。  
- 不得在本目录内直接操作会话/消息 DB 或缓存：  
  - 会话持久化与缓存更新必须通过 `ConvDatasource` 与 `ReceiveConversation` 间接完成。  
  - 消息落地必须通过 `MessageManager` 触发，保持与消息领域的一致性和可靠性约束。  
- 所有对 `SDKRoot` 的访问都必须通过 `std::weak_ptr` 安全获取，严禁在异步回调中持有裸指针。  

## 4. 局部编写规范

- 新增混链拉取策略或协议字段时：  
  - 优先在本目录内扩展请求/响应处理逻辑，避免在 `ConversationManager` 中堆叠分支。  
  - 如需更改游标语义或完整性校验流程，必须在注释中补充时序说明，并在相关文档中更新描述。  
- `FetchUserMessages` 内部循环必须始终有清晰的退出条件（例如最大轮次计数 `cnt`），避免在异常响应下出现无限循环。  
- 所有网络错误与解析错误需要通过日志记录关键上下文（用户 ID、游标区间、错误码），但禁止记录用户敏感内容。  
- 修改本目录逻辑时，务必检查与 `ConvDatasource::ChatsCursor`、`ConversationStatusHandler` 以及消息接收链路之间的一致性。  

