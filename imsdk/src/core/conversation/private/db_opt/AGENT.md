# 子目录 AGENT：`imsdk/src/core/conversation/private/db_opt`

## 1. 职责定位

- 负责会话领域在 WCDB 上的**底层数据访问与更新**：  
  - 建表、插入、查询单个/多个会话。  
  - 更新会话的置顶、免打扰、拉黑、扩展字段、本地扩展、删除状态等字段。  
  - 维护用户级别的会话游标（`ChatsCursor`），为混链拉取提供持久化水位。  
  - 在网络会话覆盖本地记录时，合并本地独有字段（如草稿、local_ext）。  

## 2. 关键类与方法

- **`DBOpt`**  
  - `CreateConversationTableIfNeed`：按用户维度创建会话表，使用 `KeyForUser` 生成表名。  
  - `InsertConversation`：事务性插入/覆盖一批 `ConversationORM`。  
  - `QueryConversations`：按照 `last_message_server_id`/时间戳等顺序分页查询会话，并通过 `Convert` 转成 `ConversationModel`。  
  - `ConversationForId`：按 `conversation_id` 查询单条会话并转换为 SDK 模型。  
  - `DeleteConversation` / `SetConversationTop` / `SetConversationMute` / `SetConversationBlock`：更新对应布尔字段并记录日志。  
  - `SetConversationSyncExt` / `SetConversationLocalExt`：从 DB 中读取现有 JSON 字段，解析为 map 合并新值后再序列化写回。  
  - `ChatsCursor` / `SetChatsCursor`：基于 MMKV 存取当前用户的会话游标。  
  - `ConversationMergeWithLocal`：在网络会话覆盖本地记录前，从 DB 查出本地独有字段（`draft`, `local_ext`）合并到新记录中。  

## 3. 线程与依赖约束

- DB 读写必须通过 `SDKRoot::database()`/`mmkv()` 获取句柄；本目录不关心线程模型，由上层（如 `ConvDatasource`）保证在合适的 `strand` 上调用。  
- 禁止在本目录内直接访问会话缓存或 SDK 模型结构体，所有 SDK 层结构转换都应通过 `Convert` 完成。  
- 所有外部调用必须通过 `CHECK_ROOT_OR_RETURN_VALUE` / `CHECK_POINTER_OR_RETURN_VALUE` 宏保护，避免在 `SDKRoot` 已销毁时访问资源。  

## 4. 局部编写规范

- 新增/修改会话字段时：  
  - 需要同步更新 `ConversationORM` 结构与 WCDB 字段/索引声明。  
  - 在本目录内提供成对的查询/更新接口，避免在其它模块中拼 SQL 或操作 ORM 字段。  
- JSON map 字段（如 `sync_ext` / `local_ext`）的读写必须通过 `json_util` 完成：  
  - 解析失败时记录日志并回退到空 map，不得直接抛出异常给上层。  
  - 合并策略必须清晰（同 key 以新写入值覆盖旧值）。  
- 所有更新类接口在失败时必须返回 `false` 并打出包含会话 ID 与错误上下文的日志，便于排查 DB 问题。  

