# 目录 AGENT：`imsdk/src/core/conversation`

> 本文件只负责「会话领域目录」的**总览与分工**，具体类/模块的细节请看各子目录的 `AGENT.md`。

## 1. 目录职责与边界

- **整体定位**  
  - 会话领域的根目录，负责「会话列表与会话元数据」相关能力：会话创建、首屏加载、分页加载、状态更新（置顶、免打扰、拉黑、删除）、扩展字段与草稿等。  
  - 对上：通过 `roc::imsdk::core::ConversationManager` 和 `model::ConversationModel` 对 SDK 其它模块/业务层暴露统一的会话接口。  
  - 对下：将实现拆分到多个子模块目录（`private/`、`db_model/` 等），各自用 `AGENT.md` 说明职责与局部约束。  
- **不负责的内容**  
  - 单条消息的内容读写、消息状态机与消息可靠性（由 `imsdk/src/core/message/` 负责）。  
  - 具体网络连接与请求发送细节（由 `ConnectionManager` 等网络层负责）。  
  - UI 展示以及业务层对会话的排序策略（仅提供必要字段与约束）。  

## 2. 子目录划分

> 约定：每个子目录都需要有自己的 `AGENT.md`，描述该子模块下类/函数的职责与局部规范。

- **根目录文件**  
  - `ConversationManager.*`：会话领域的总入口，对外暴露查询/更新类 API，并组合下述子模块。  

- **`private/datasource/`**  
  - 负责本地会话数据源：从 DB 读写会话、维护会话缓存、根据游标分页返回 `ConversationModel`，详见 `private/datasource/AGENT.md`。  

- **`private/db_opt/`**  
  - 负责会话相关的 DB 操作封装（基于 WCDB）：建表、插入、查询、局部字段更新、游标持久化等，详见 `private/db_opt/AGENT.md`。  

- **`private/receive/`**  
  - 负责处理从服务器下发的会话列表/增量会话：调用 `ConvDatasource` 落库并通过回调对外上抛，详见 `private/receive/AGENT.md`。  

- **`private/convert/`**  
  - 负责会话在协议模型 `network::ConversationData`、ORM 模型 `ConversationORM` 与 SDK 模型 `ConversationModel` 之间的转换，并合并本地独有字段，详见 `private/convert/AGENT.md`。  

- **`private/fetcher/`**  
  - 负责用户登录或重进时的混链会话/消息拉取：维护会话游标、发起网络请求、校验完整性并驱动消息与会话落地，详见 `private/fetcher/AGENT.md`。  

- **`private/handler/`**  
  - 负责会话状态类操作的编排与 CMD 处理：置顶、免打扰、拉黑、扩展字段、删除、群聊创建与邀请等，详见 `private/handler/AGENT.md`。  

- **`db_model/`**  
  - 存放会话相关的 ORM 模型定义（如 `ConversationORM`），约定主键/索引与字段含义，详见 `db_model/AGENT.md`。  

## 3. 本目录编写规范（局部约束）

- **根目录 AGENT 的角色**  
  - 只维护「子模块划分 + 依赖关系」的高层说明，不展开每个类的具体实现细节。  
  - 当增加新的子模块目录时，必须同步在此文件中补充一行说明，并在该目录下创建对应的 `AGENT.md`。  

- **依赖方向**  
  - `ConversationManager` 可以依赖各个 `private/*` 子模块与 `db_model/`，但不得直接依赖消息领域的内部实现（通过 `MessageManager` 间接协作）。  
  - 子模块之间如需依赖，应通过清晰的头文件接口，避免出现复杂环状依赖；跨子模块依赖需要在各自 `AGENT.md` 中注明。  

- **线程与执行上下文**  
  - 会话领域的 DB 与缓存更新统一在 `ConvDatasource::ConvStrand()` 对应的 `sdk_io_context` 串行执行，禁止在外部线程直接操作会话缓存或 ORM。  
  - 网络请求与完整性校验通过 `SDKRoot::net_io_context()` 触发，结果通过 `sdk_io_context` 回到领域层，必须遵守宪章中线程模型相关约束。  

- **跨领域协作**  
  - 会话领域如需访问消息内容，只能通过 `MessageManager` 或消息领域公开的接口获取，不得越层直接操作消息 DB 或内部模型。  
  - 新增能力时，优先在本目录/子目录扩展逻辑，不要在上层对外 API 中堆积过多条件分支。  

