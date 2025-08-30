# ROC IM SDK - spdlog 日志系统集成

本项目已集成 [spdlog](https://github.com/gabime/spdlog) 作为日志系统，提供高性能、异步的 C++ 日志记录功能。

## 安装 spdlog

### 方法 1: 使用 vcpkg (推荐)

由于项目已配置 vcpkg，可以直接安装 spdlog：

```bash
# 安装 spdlog
vcpkg install spdlog

# 或者安装特定版本
vcpkg install spdlog:x64-linux
```

### 方法 2: 手动安装

如果不想使用 vcpkg，可以手动安装：

```bash
# 克隆 spdlog 仓库
git clone https://github.com/gabime/spdlog.git
cd spdlog

# 创建构建目录
mkdir build && cd build

# 配置和构建
cmake .. -DSPDLOG_BUILD_SHARED=ON -DSPDLOG_ENABLE_PCH=ON
make -j$(nproc)
sudo make install
```

## 项目配置

项目已在 `CMakeLists.txt` 中配置了 spdlog 依赖：

```cmake
# SPDLOG 库
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(main PRIVATE spdlog::spdlog)
```

## 使用方法

### 1. 基本使用

```cpp
#include "base/Logger.h"

int main() {
    // 初始化日志系统
    roc_im_sdk::Logger::Initialize("logs", roc_im_sdk::Logger::Level::Debug);
    
    // 使用日志宏
    LOG_INFO("应用启动成功");
    LOG_DEBUG("调试信息: {}", "详细信息");
    LOG_WARN("警告: 网络连接不稳定");
    LOG_ERROR("错误: 数据库连接失败");
    
    // 清理
    roc_im_sdk::Logger::Shutdown();
    return 0;
}
```

### 2. 模块化日志

```cpp
// 为不同模块创建专门的日志器
LOG_MODULE_INFO("Network", "建立 WebSocket 连接");
LOG_MODULE_DEBUG("Database", "执行查询: {}", sql);
LOG_MODULE_ERROR("Auth", "用户认证失败: {}", userId);
```

### 3. 高级功能

```cpp
// 设置日志级别
roc_im_sdk::Logger::SetLevel(roc_im_sdk::Logger::Level::Trace);

// 自定义日志格式
roc_im_sdk::Logger::SetPattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");

// 获取特定日志器
auto network_logger = roc_im_sdk::Logger::GetLogger("network");
network_logger->info("网络模块日志");
```

## 日志级别

spdlog 支持以下日志级别（从低到高）：

- `Trace` - 最详细的调试信息
- `Debug` - 调试信息
- `Info` - 一般信息
- `Warn` - 警告信息
- `Error` - 错误信息
- `Critical` - 严重错误信息
- `Off` - 关闭所有日志

## 日志输出

### 控制台输出
- 支持彩色输出
- 可配置是否启用

### 文件输出
- **轮转文件**: `roc_im_sdk.log` (10MB 轮转，保留 5 个文件)
- **每日文件**: `roc_im_sdk_daily.log` (按日期分割)

## 性能特性

- **异步日志**: 支持异步日志记录，不阻塞主线程
- **高性能**: 基于 fmt 库的高性能格式化
- **内存效率**: 最小化内存分配
- **线程安全**: 完全线程安全

## 示例代码

查看 `examples/spdlog_example.cpp` 文件获取完整的使用示例。

## 编译

确保已安装 spdlog 后，正常编译项目：

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 故障排除

### 1. 找不到 spdlog 头文件

确保 spdlog 已正确安装：

```bash
# 检查 vcpkg 安装
vcpkg list | grep spdlog

# 重新安装
vcpkg remove spdlog
vcpkg install spdlog
```

### 2. 链接错误

检查 CMake 配置是否正确：

```bash
# 清理构建目录
rm -rf build
mkdir build && cd build

# 重新配置
cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 3. 运行时错误

检查日志目录权限：

```bash
# 确保有写入权限
chmod 755 logs/
```

## 更多信息

- [spdlog 官方文档](https://github.com/gabime/spdlog)
- [spdlog 示例](https://github.com/gabime/spdlog/tree/v1.x/example)
- [fmt 格式化语法](https://fmt.dev/latest/syntax.html)

## 许可证

spdlog 使用 MIT 许可证，详见 [LICENSE](https://github.com/gabime/spdlog/blob/v1.x/LICENSE) 文件。
