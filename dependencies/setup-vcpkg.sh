#!/bin/bash
# 为 ROCIM 项目准备 vcpkg（与 vcpkg.json builtin-baseline 一致，保证 protobuf 3.5.1 可构建）
# 依赖相关文件统一放在 dependencies 目录。
# 使用: 在仓库目录执行 ./dependencies/setup-vcpkg.sh

set -e

# 与 dependencies/vcpkg.json 中 builtin-baseline 保持一致
VCPKG_BASELINE="df8bfe519564ae001903e5cdd32af0999531ef71"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPENDENCIES_DIR="$PROJECT_ROOT/dependencies"
VCPKG_DIR="$DEPENDENCIES_DIR/vcpkg"

echo "[INFO] 仓库目录: $PROJECT_ROOT"
echo "[INFO] 依赖目录: $DEPENDENCIES_DIR"
echo "[INFO] 目标 vcpkg 目录: $VCPKG_DIR"
echo "[INFO] 使用 baseline: $VCPKG_BASELINE（与 dependencies/vcpkg.json 一致）"
echo ""

if [ -d "$VCPKG_DIR" ]; then
    echo "[INFO] 已存在 vcpkg 目录，尝试切换到 baseline 提交..."
    cd "$VCPKG_DIR"
    if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        echo "[ERROR] $VCPKG_DIR 不是 git 仓库，请删除后重新运行此脚本"
        exit 1
    fi
    git fetch origin 2>/dev/null || true
    git checkout "$VCPKG_BASELINE" 2>/dev/null || {
        echo "[WARN] 无法 checkout $VCPKG_BASELINE，若当前已在其他提交，protobuf 3.5.1 可能构建失败"
    }
else
    echo "[INFO] 克隆 vcpkg 并切换到 baseline..."
    mkdir -p "$DEPENDENCIES_DIR"
    cd "$DEPENDENCIES_DIR"
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    cd vcpkg
    git checkout "$VCPKG_BASELINE"
fi

if [ ! -x "$VCPKG_DIR/vcpkg" ]; then
    echo "[INFO] 执行 bootstrap-vcpkg.sh..."
    "$VCPKG_DIR/bootstrap-vcpkg.sh"
fi

echo ""
echo "[SUCCESS] vcpkg 已就绪（baseline: $VCPKG_BASELINE）"
echo "[INFO] 安装依赖请执行:"
echo "  ./dependencies/install-with-vcpkg.sh"
echo "  或显式指定: ./dependencies/install-with-vcpkg.sh --vcpkg-root $VCPKG_DIR"
echo ""
