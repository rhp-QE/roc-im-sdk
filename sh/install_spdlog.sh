#!/bin/bash

# ROC IM SDK - spdlog 安装脚本
# 支持 vcpkg 和手动安装两种方式

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查命令是否存在
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 检查 vcpkg 是否可用
check_vcpkg() {
    if [ -f "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        return 0
    elif [ -f "$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        return 0
    else
        return 1
    fi
}

# 使用 vcpkg 安装 spdlog
install_with_vcpkg() {
    print_info "使用 vcpkg 安装 spdlog..."
    
    if ! command_exists vcpkg; then
        print_error "vcpkg 命令未找到，请先安装 vcpkg"
        return 1
    fi
    
    # 检查是否已安装
    if vcpkg list | grep -q "spdlog"; then
        print_warning "spdlog 已安装，版本: $(vcpkg list | grep spdlog)"
        return 0
    fi
    
    # 安装 spdlog
    print_info "正在安装 spdlog..."
    if vcpkg install spdlog; then
        print_success "spdlog 安装成功！"
        return 0
    else
        print_error "spdlog 安装失败"
        return 1
    fi
}

# 手动安装 spdlog
install_manually() {
    print_info "手动安装 spdlog..."
    
    # 检查依赖
    if ! command_exists git; then
        print_error "git 未安装，请先安装 git"
        return 1
    fi
    
    if ! command_exists cmake; then
        print_error "cmake 未安装，请先安装 cmake"
        return 1
    fi
    
    if ! command_exists make; then
        print_error "make 未安装，请先安装 make"
        return 1
    fi
    
    # 创建临时目录
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    print_info "克隆 spdlog 仓库..."
    if git clone https://github.com/gabime/spdlog.git; then
        print_success "仓库克隆成功"
    else
        print_error "仓库克隆失败"
        cd - > /dev/null
        rm -rf "$TEMP_DIR"
        return 1
    fi
    
    cd spdlog
    
    # 创建构建目录
    mkdir -p build
    cd build
    
    print_info "配置 CMake..."
    if cmake .. -DSPDLOG_BUILD_SHARED=ON -DSPDLOG_ENABLE_PCH=ON -DCMAKE_BUILD_TYPE=Release; then
        print_success "CMake 配置成功"
    else
        print_error "CMake 配置失败"
        cd - > /dev/null
        rm -rf "$TEMP_DIR"
        return 1
    fi
    
    print_info "编译 spdlog..."
    if make -j$(nproc); then
        print_success "编译成功"
    else
        print_error "编译失败"
        cd - > /dev/null
        rm -rf "$TEMP_DIR"
        return 1
    fi
    
    print_info "安装 spdlog..."
    if sudo make install; then
        print_success "安装成功"
    else
        print_error "安装失败"
        cd - > /dev/null
        rm -rf "$TEMP_DIR"
        return 1
    fi
    
    # 更新动态链接库缓存
    if command_exists ldconfig; then
        print_info "更新动态链接库缓存..."
        sudo ldconfig
    fi
    
    # 清理临时文件
    cd - > /dev/null
    rm -rf "$TEMP_DIR"
    
    return 0
}

# 验证安装
verify_installation() {
    print_info "验证 spdlog 安装..."
    
    # 创建测试文件
    cat > /tmp/test_spdlog.cpp << 'EOF'
#include <spdlog/spdlog.h>
int main() {
    spdlog::info("Hello from spdlog!");
    return 0;
}
EOF
    
    # 尝试编译
    if g++ -std=c++17 -o /tmp/test_spdlog /tmp/test_spdlog.cpp -lspdlog; then
        print_success "spdlog 编译测试通过"
        
        # 运行测试
        if /tmp/test_spdlog; then
            print_success "spdlog 运行测试通过"
        else
            print_warning "spdlog 运行测试失败"
        fi
        
        # 清理测试文件
        rm -f /tmp/test_spdlog /tmp/test_spdlog.cpp
        return 0
    else
        print_error "spdlog 编译测试失败"
        rm -f /tmp/test_spdlog /tmp/test_spdlog.cpp
        return 1
    fi
}

# 主函数
main() {
    echo "🚀 ROC IM SDK - spdlog 安装脚本"
    echo "=================================="
    
    # 检查是否以 root 身份运行
    if [ "$EUID" -eq 0 ]; then
        print_warning "不建议以 root 身份运行此脚本"
        read -p "是否继续？(y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    # 检查 vcpkg 可用性
    if check_vcpkg; then
        print_info "检测到 vcpkg 配置"
        if install_with_vcpkg; then
            print_success "vcpkg 安装完成"
        else
            print_warning "vcpkg 安装失败，尝试手动安装"
            if install_manually; then
                print_success "手动安装完成"
            else
                print_error "所有安装方式都失败了"
                exit 1
            fi
        fi
    else
        print_info "未检测到 vcpkg 配置，使用手动安装"
        if install_manually; then
            print_success "手动安装完成"
        else
            print_error "手动安装失败"
            exit 1
        fi
    fi
    
    # 验证安装
    if verify_installation; then
        print_success "🎉 spdlog 安装和验证完成！"
        echo
        echo "接下来你可以："
        echo "1. 编译项目: mkdir build && cd build && cmake .. && make"
        echo "2. 查看使用说明: cat README_SPDLOG.md"
        echo "3. 运行示例: ./examples/spdlog_example"
    else
        print_error "❌ spdlog 验证失败，请检查安装"
        exit 1
    fi
}

# 运行主函数
main "$@"
