#!/bin/bash

# ROCIM vcpkg 依赖安装脚本
# 所有依赖（包括 MMKV 和 WCDB）通过 vcpkg 统一管理

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

show_help() {
    echo "ROCIM vcpkg 依赖安装脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help             显示此帮助信息"
    echo "  --vcpkg-root PATH      指定 vcpkg 根目录（默认: /opt/vcpkg）"
    echo ""
    echo "示例:"
    echo "  $0                      # 安装所有依赖"
    echo "  $0 --vcpkg-root /opt/vcpkg  # 指定 vcpkg 路径"
    echo ""
    echo "安装完成后，使用以下命令编译项目:"
    echo "  cd roc-im-sdk && ./build clean"
}


# 检查 vcpkg
check_vcpkg() {
    local vcpkg_root=$1
    
    print_info "检查 vcpkg..."
    
    if [ ! -d "$vcpkg_root" ]; then
        print_error "vcpkg 目录不存在: $vcpkg_root"
        print_info "请安装 vcpkg:"
        echo "  sudo mkdir -p /opt"
        echo "  cd /opt"
        echo "  sudo git clone https://github.com/microsoft/vcpkg.git"
        echo "  cd vcpkg"
        echo "  sudo ./bootstrap-vcpkg.sh"
        exit 1
    fi
    
    if [ ! -x "$vcpkg_root/vcpkg" ]; then
        print_error "vcpkg 可执行文件不存在或没有执行权限"
        print_info "请运行: cd $vcpkg_root && ./bootstrap-vcpkg.sh"
        exit 1
    fi
    
    print_success "vcpkg 已就绪: $vcpkg_root"
}

# 安装依赖（Manifest 模式 - 主流方式）
install_dependencies() {
    local vcpkg_root=$1
    
    print_info "=========================================="
    print_info "安装 vcpkg 依赖（Manifest 模式）..."
    print_info "=========================================="
    print_info ""
    print_info "依赖将安装到: $PROJECT_ROOT/vcpkg_installed/x64-linux/"
    print_info "使用 manifest 文件: $PROJECT_ROOT/vcpkg.json"
    print_info ""
    
    # 设置 VCPKG_ROOT 环境变量
    export VCPKG_ROOT="$vcpkg_root"
    
    # 切换到项目目录
    cd "$PROJECT_ROOT"
    
    # Manifest 模式自动读取 vcpkg.json 和 overlay-ports
    "$vcpkg_root/vcpkg" install --triplet=x64-linux
    
    if [ $? -eq 0 ]; then
        print_success "所有依赖安装到项目目录：$PROJECT_ROOT/vcpkg_installed/"
    else
        print_error "vcpkg 依赖安装失败"
        exit 1
    fi
}

# 主函数
main() {
    local vcpkg_root="/opt/vcpkg"
    
    export PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            --vcpkg-root)
                if [[ -n $2 ]]; then
                    vcpkg_root=$2
                    shift 2
                else
                    print_error "请指定 vcpkg 根目录"
                    exit 1
                fi
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    echo ""
    echo "========================================="
    echo "ROCIM vcpkg 依赖安装"
    echo "========================================="
    echo ""
    print_info "项目根目录: $PROJECT_ROOT"
    print_info "vcpkg 路径: $vcpkg_root"
    echo ""
    
    # 检查 vcpkg
    check_vcpkg "$vcpkg_root"
    echo ""
    
    # 安装依赖
    install_dependencies "$vcpkg_root"
    echo ""
    
    echo "========================================="
    print_success "依赖安装完成！"
    echo "========================================="
    echo ""
    print_info "所有依赖已安装到: $PROJECT_ROOT/vcpkg_installed/"
    echo ""
    print_info "下一步：编译项目"
    echo "  cd roc-im-sdk && ./build clean"
    echo ""
    print_info "或者使用其他构建选项："
    echo "  ./build          # 普通构建"
    echo "  ./build debug    # Debug 模式"
    echo "  ./build run      # 构建并运行"
    echo ""
    echo "========================================="
}

# 运行主函数
main "$@"

