# 目录 AGENT：`imsdk/src/core/message`

> 本文件只负责「消息领域目录」的**总览与分工**，具体类/模块的细节请看各子目录的 `AGENT.md`。

## 1. 目录职责与边界

- **整体定位**：  
  - 消息领域的根目录，负责「消息」相关的所有领域能力：发送、接收、状态变更、本地持久化、缓存与会话消息拉取。  
  - 对上：通过 `roc::imsdk::core::MessageManager` 对 SDK 其它模块暴露统一的消息接口。  
  - 对下：将实现拆分到多个子模块目录（`send/`、`receive/`、`data_source/` 等），各自用 `AGENT.md` 说明职责。  
- **不负责的内容**：  
  - UI/业务场景（如会话列表、草稿管理等）。  
  - 网络连接管理（如长连接建立/重连）与底层 DB 驱动的具体实现。  

## 2. 子目录划分

> 约定：每个子目录都需要有自己的 `AGENT.md`，描述该子模块下类/函数的职责与局部规范。

- **根目录文件**  
  - `MessageManager.*`：消息领域的总入口，对外暴露 API，并组合各子模块。  

- **`private/send/`**  
  - 负责单条消息发送链路的完整编排（`SendMessageController` 等），详见 `private/send/AGENT.md`。  

- **`private/receive/`**  
  - 负责从网络层接收消息并落地、触发回调，详见 `private/receive/AGENT.md`。  

- **`private/data_source/`**  
  - 负责本地消息的 DB 读写、缓存维护与消息区间管理，详见 `private/data_source/AGENT.md`。  

- **`private/db_opt/`**  
  - 负责消息相关的 DB 操作封装（基于 WCDB），详见 `private/db_opt/AGENT.md`。  

- **`private/convert/`**  
  - 负责协议模型 / ORM / SDK 模型之间的转换逻辑，详见 `private/convert/AGENT.md`。  

- **`private/fetcher/`**  
  - 负责进入会话时的消息拉取与补齐策略，详见 `private/fetcher/AGENT.md`。  

- **`private/handler/`**  
  - 负责消息状态变更（删除、撤回、置顶、扩展字段变更等），详见 `private/handler/AGENT.md`。  

- **`db_model/`**  
  - 存放消息相关的 ORM 模型定义（如 `MessageORM`），详见 `db_model/AGENT.md`。  

- **`private/common/`**  
  - 存放消息领域内部使用的公共模型/工具（如 `model.h`），详见 `private/common/AGENT.md`。  

## 3. 本目录编写规范（局部约束）

- **根目录 AGENT 的角色**  
  - 只维护「子模块划分 + 依赖关系」的高层说明，不再展开每个类的细节。  
  - 当增加新的子模块目录时，必须同步在此文件中补充一行说明，并在该目录下创建对应的 `AGENT.md`。  

- **依赖方向**  
  - 根目录（`MessageManager`）可以依赖各个 `private/*` 子模块与 `db_model/`。  
  - 子模块之间如需依赖，应通过清晰的头文件接口，避免出现复杂的环状依赖；跨子模块依赖也需要在各自 `AGENT.md` 中注明。  


