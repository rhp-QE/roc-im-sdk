#!/bin/bash

# 使用 Clang 编译 ROC IM SDK
# 支持 C++23 标准

set -e

echo "🚀 Building ROC IM SDK with Clang and C++23 support..."

# 检查 Clang 是否安装
if ! command -v clang++ &> /dev/null; then
    echo "❌ Error: clang++ not found. Please install Clang first."
    echo "   Ubuntu/Debian: sudo apt install clang"
    echo "   CentOS/RHEL: sudo yum install clang"
    echo "   macOS: xcode-select --install"
    exit 1
fi

# 检查 Clang 版本
CLANG_VERSION=$(clang++ --version | head -n1)
echo "🔧 Using Clang: $CLANG_VERSION"

# 创建构建目录
BUILD_DIR="build_clang"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake 项目
echo "🔧 Configuring CMake project..."
cmake .. \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_CXX_STANDARD=23 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DCMAKE_BUILD_TYPE=Debug

# 编译项目
echo "🔧 Building project..."
make -j$(nproc)

echo "✅ Build completed successfully!"
echo "🚀 Executable location: $BUILD_DIR/main"
