# ROCIM SDK（AI 使用说明）

本文面向 AI Agent，当前仅定义“如何构建”。
后续会新增“如何使用 SDK”章节。

## 如何构建

项目根目录为 `roc-im-sdk`，以下命令均在项目根目录下执行：

```bash
./dependencies/setup-vcpkg.sh
./dependencies/install-with-vcpkg.sh
./build_ninja.sh
```

> 说明：`protobuf` 固定为 `3.5.1`，`WCDB` 使用脚本内锁定的稳定版本。  
> 构建产物位于仓库内 `build/`，`compile_commands.json` 输出到仓库根目录。
