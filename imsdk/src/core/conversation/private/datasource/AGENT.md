# 子目录 AGENT：`imsdk/src/core/conversation/private/datasource`

## 1. 职责定位

- 负责**本地会话数据源**：  
  - 从 DB 读取会话，并维护内存中的会话缓存。  
  - 提供按游标/限制数量分页加载会话的能力（用于首屏和后续上拉加载）。  
  - 提供按会话 ID 查询单个 `ConversationModel` 的能力，优先走缓存，必要时回落到 DB。  
  - 在 DB 更新后同步刷新缓存，确保对外读取到的会话视图始终一致。  

## 2. 关键类

- **`ConvDatasource`**  
  - `SaveNetConversations`：将网络层下发的 `ConversationData` 转换为 `ConversationORM` 与 `ConversationModel`，在 `ConvStrand` 上统一完成插入 DB 与更新缓存。  
  - `LoadConvsFromDb`：基于游标从 DB 查询会话，更新缓存后组装为 `LoadUserConvsResult` 返回。  
  - `SdkConvForId` / `SdkConvForIdFromCache`：按会话 ID 从缓存/DB 获取单个会话视图。  
  - 一系列 `UpdateConversation*Status` 方法：在 `ConvStrand` 上以「先 DB 后缓存」的方式更新置顶、免打扰、拉黑、扩展字段、删除等状态。  
  - `p_UpdateConvCache`：统一维护会话缓存，将 `ConversationModel` 与最后一条消息 `MessageModel` 绑定。  

## 3. 线程与依赖约束

- 所有会话 DB 写入与缓存更新必须在 `ConvStrand()` 对应的 `sdk_io_context` 串行执行：  
  - 外部调用只允许通过公开的 `awaitable` 接口，禁止直接在其它线程修改 `conv_cache_`。  
  - `co_spawn(ConvStrand(), ...)` 的 lambda 内部必须再次校验 `SDKRoot` 是否仍然有效。  
- `ConvDatasource` 只能通过 `ConversationManager` 获取 `DBOpt` 与 `Convert` 等依赖，不得直接创建或持有 DB 连接。  
- 获取最后一条消息内容时，必须通过 `MessageManager::MessageForId` 协作，禁止跨领域直接访问消息 DB。  

## 4. 局部编写规范

- 新增任何写 DB 的会话相关接口：  
  - 必须在 `ConvStrand` 上执行，保持与现有更新逻辑相同的串行语义。  
  - 必须遵循「DB 更新成功后再尝试更新缓存」的顺序，避免缓存与 DB 长期不一致。  
- 修改缓存结构或 key（例如会话 ID）时：  
  - 需要审视所有 `conv_cache_.at(...)` 调用路径，并在注释中说明缓存失效和重建策略。  
  - 确保不会在热路径上产生过多拷贝或锁竞争。  
- 与会话分页策略（游标、排序字段）相关的改动，需要同步检查 `DBOpt::QueryConversations` 的排序与索引约束，避免出现查询与缓存排序不一致的问题。  

