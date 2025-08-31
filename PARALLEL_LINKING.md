# 🚀 并行链接配置指南

本项目已配置支持并行链接，可以显著提升大型项目的链接速度。

## 🎯 **快速开始**

**并行链接功能已完全集成到 `build` 脚本中，无需额外配置！**

```bash
# 直接构建，并行链接自动启用
./build rc

# 查看并行链接配置
./build parallel
```

## 🔧 支持的链接器

### 1. Mold 链接器（推荐）
- **优势**: 原生支持并行链接，速度最快
- **安装**: `sudo apt install mold`
- **特点**: 内存占用低，并行效率高
- **并行控制**: 自动检测 CPU 核心数，无需 `NINJA_LINK_JOBS` 设置

### 2. Gold 链接器（备选）
- **优势**: GCC 的现代链接器，支持并行链接
- **特点**: 系统自带，无需额外安装

### 3. 默认链接器
- **特点**: 使用系统默认链接器，并行支持有限

## 🚀 启用并行链接

### 方法 1: 自动启用（推荐）

并行链接功能已集成到 `build` 脚本中，**无需额外设置**！

```bash
# 直接构建，并行链接自动启用
./build rc

# 或使用其他命令
./build build
./build clean
./build run
```

### 方法 2: 查看并行链接配置

```bash
# 显示当前并行链接配置
./build parallel
```

### 方法 3: 手动设置环境变量（高级用户）

```bash
# 设置编译并行度
export NINJA_JOBS=$(nproc)

# 设置链接并行度（建议不超过2，避免内存不足）
export NINJA_LINK_JOBS=2

# 设置链接器并行参数
export LDFLAGS="-Wl,--parallel"

# 构建项目
./build rc
```

### 方法 4: 在构建命令中直接设置

```bash
# 一次性设置并构建
NINJA_JOBS=$(nproc) NINJA_LINK_JOBS=2 LDFLAGS="-Wl,--parallel" ./build rc
```

## 📊 性能提升

| 链接器 | 并行度 | 预期性能提升 |
|--------|--------|--------------|
| Mold   | 2-4    | 3-5x        |
| Gold   | 2-4    | 2-3x        |
| 默认   | 1      | 无提升      |

## ⚠️ 注意事项

### 内存使用
- 并行链接会增加内存使用
- 对于 Gold 链接器，建议 `NINJA_LINK_JOBS` 不超过 2
- 对于 Mold 链接器，无需设置 `NINJA_LINK_JOBS`，会自动优化
- 如果遇到内存不足，减少并行度

### 链接器差异
- **Mold**: 原生并行支持，自动使用 CPU 核心，无需额外配置
- **Gold**: 需要 `NINJA_LINK_JOBS` 参数控制并行度
- **默认链接器**: 并行支持有限，性能提升不明显

### 系统兼容性
- Mold: Ubuntu 20.04+, Debian 11+
- Gold: GCC 4.1.2+
- 某些系统可能需要更新包管理器

## 🔍 验证配置

构建时查看以下输出确认并行链接已启用：

```
✅ Found Mold linker: /usr/bin/mold
🔧 Mold linker enabled for faster linking
🔧 Parallel linking enabled
🚀 Ninja parallel jobs: 8
🚀 Ninja parallel link jobs: 2
🚀 Linker: Mold (/usr/bin/mold)
```

## 🛠️ 故障排除

### 问题 1: 链接器未找到
```bash
# 安装 mold
sudo apt update && sudo apt install mold

# 或使用 gold
sudo apt install binutils-gold
```

### 问题 2: 内存不足
```bash
# 减少链接并行度
export NINJA_LINK_JOBS=1
```

### 问题 3: 链接失败
```bash
# 回退到默认链接器
unset LDFLAGS
unset MOLD_FLAGS
```

## 📝 永久配置

要永久启用并行链接，将以下内容添加到 `~/.bashrc` 或 `~/.zshrc`：

```bash
# 并行链接配置
export NINJA_JOBS=$(nproc)
export NINJA_LINK_JOBS=2
export LDFLAGS="-Wl,--parallel"

# 如果使用 mold
if command -v mold >/dev/null 2>&1; then
    export MOLD_FLAGS="-Wl,--parallel"
fi
```

然后重新加载配置：
```bash
source ~/.bashrc
```

## 🎯 最佳实践

1. **优先使用 Mold**: 性能最佳，内存占用低
2. **合理设置并行度**: 链接并行度通常不超过 2
3. **监控内存使用**: 避免因并行链接导致内存不足
4. **测试验证**: 在重要构建前测试并行链接配置

---

💡 **提示**: 并行链接对大型项目效果最明显，小型项目可能提升有限。
