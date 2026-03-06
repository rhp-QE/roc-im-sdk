# 子目录 AGENT：`imsdk/src/core/message/private/data_source`

## 1. 职责定位

- 负责**本地消息数据源**：  
  - 消息在本地的生命周期管理（写入 DB、更新缓存、按 ID/会话查询）。  
  - 维护会话内消息顺序与区间（server/client order index 与缺失区间）。  

## 2. 关键类

- **`MessageDataSource`**  
  - `SaveDbMsgs` / `SaveNetMessages`：统一入口，将消息转成 ORM + SDK 模型并在单一 `msg_strand_` 上完成 DB 写入与缓存更新。  
  - `SdkMsgForId` / `LoadMessageFromDb`：读取当前会话/单条消息的本地视图，并更新缓存。  
  - 一系列 `UpdateMessage*Status` 方法，用于以 DB 为主、缓存为辅地更新 pinned/sync_ext/property/local_ext/deleted/recalled 等状态。  

## 3. 线程与依赖约束

- `msg_strand_` 必须用于所有写 DB + 写缓存的路径，确保串行执行。  
- 只能通过 `MessageManager` 间接访问 `DBOpt` 与其它消息子模块。  

## 4. 局部编写规范

- 新增任何写 DB 的接口。  
  - 必须在 `msg_strand_` 内完成 DB 与缓存两步操作。  
  - 必须保证更新后的缓存状态可被其它查询 API 一致地读到。  
- 与消息区间/顺序相关的算法（如 `p_generateRange`, `p_mergeRanges`）若有修改，应在注释与测试中说明边界条件。  

