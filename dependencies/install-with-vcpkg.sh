#!/bin/bash

# ROCIM vcpkg 依赖安装脚本
# 所有依赖（包括 MMKV）统一安装到 dependencies 目录下

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
    echo "  --vcpkg-root PATH      指定 vcpkg 根目录（默认: <仓库目录>/dependencies/vcpkg）"
    echo ""
    echo "示例:"
    echo "  $0                           # 安装 vcpkg 依赖并编译安装 WCDB"
    echo "  $0 --vcpkg-root ./dependencies/vcpkg"
    echo ""
}

# 检查 vcpkg
check_vcpkg() {
    local vcpkg_root=$1

    print_info "检查 vcpkg..."

    if [ ! -d "$vcpkg_root" ]; then
        print_error "vcpkg 目录不存在: $vcpkg_root"
        print_info "请先在仓库目录执行: ./dependencies/setup-vcpkg.sh"
        exit 1
    fi

    if [ ! -x "$vcpkg_root/vcpkg" ]; then
        print_error "vcpkg 可执行文件不存在或没有执行权限"
        print_info "请运行: cd $vcpkg_root && ./bootstrap-vcpkg.sh"
        exit 1
    fi

    print_success "vcpkg 已就绪: $vcpkg_root"
}

# 安装依赖（Manifest 模式）
install_dependencies() {
    local vcpkg_root=$1

    print_info "=========================================="
    print_info "安装 vcpkg 依赖（Manifest 模式）..."
    print_info "=========================================="
    print_info ""
    print_info "依赖目录: $DEPENDENCIES_DIR"
    print_info "使用 manifest 文件: $DEPENDENCIES_DIR/vcpkg.json"
    print_info "安装输出目录: $DEPENDENCIES_DIR/vcpkg_installed/x64-linux/"
    print_info ""

    export VCPKG_ROOT="$vcpkg_root"

    "$vcpkg_root/vcpkg" install \
        --triplet=x64-linux \
        --x-manifest-root="$DEPENDENCIES_DIR" \
        --x-install-root="$DEPENDENCIES_DIR/vcpkg_installed"

    if [ $? -eq 0 ]; then
        print_success "所有依赖安装到目录：$DEPENDENCIES_DIR/vcpkg_installed/"
    else
        print_error "vcpkg 依赖安装失败"
        exit 1
    fi
}

install_wcdb() {
    local wcdb_script="$DEPENDENCIES_DIR/install-wcdb.sh"

    if [ ! -x "$wcdb_script" ]; then
        print_error "WCDB 安装脚本不存在或不可执行: $wcdb_script"
        print_info "请检查 dependencies/install-wcdb.sh 是否存在"
        exit 1
    fi

    print_info "=========================================="
    print_info "安装 WCDB（源码编译）..."
    print_info "=========================================="
    "$wcdb_script"
    print_success "WCDB 安装完成"
}

sync_protoc_binary() {
    local install_root="$DEPENDENCIES_DIR/vcpkg_installed/x64-linux"
    local target_bin="$install_root/bin"
    local target_debug_bin="$install_root/debug/bin"
    local protoc_src=""
    local candidate=""
    local candidates=(
        "$install_root/tools/protoc.exe"
        "$install_root/tools/protoc"
        "$install_root/tools/protobuf/protoc"
        "$install_root/tools/protobuf/protoc.exe"
        "$install_root/bin/protoc"
        "$install_root/bin/protoc.exe"
    )

    for candidate in "${candidates[@]}"; do
        if [ -f "$candidate" ]; then
            protoc_src="$candidate"
            break
        fi
    done

    if [ -z "$protoc_src" ]; then
        print_warning "未找到 protoc 可执行文件，跳过 bin/debug/bin 同步"
        return 0
    fi

    mkdir -p "$target_bin" "$target_debug_bin"

    # 统一产出无后缀命名，兼容 Linux 构建脚本调用
    cp -f "$protoc_src" "$target_bin/protoc"
    cp -f "$protoc_src" "$target_debug_bin/protoc"
    chmod +x "$target_bin/protoc" "$target_debug_bin/protoc" || true

    # 同时保留 .exe 命名，兼容历史脚本
    cp -f "$protoc_src" "$target_bin/protoc.exe"
    cp -f "$protoc_src" "$target_debug_bin/protoc.exe"
    chmod +x "$target_bin/protoc.exe" "$target_debug_bin/protoc.exe" || true

    print_success "已同步 protoc 到:"
    print_info "  $target_bin/protoc(.exe)"
    print_info "  $target_debug_bin/protoc(.exe)"
}

# 主函数
main() {
    export PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
    export DEPENDENCIES_DIR="$PROJECT_ROOT/dependencies"
    local vcpkg_root="$DEPENDENCIES_DIR/vcpkg"

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
    print_info "依赖目录: $DEPENDENCIES_DIR"
    print_info "vcpkg 路径: $vcpkg_root"
    echo ""

    check_vcpkg "$vcpkg_root"
    echo ""

    install_dependencies "$vcpkg_root"
    echo ""

    sync_protoc_binary
    echo ""

    install_wcdb
    echo ""

    echo "========================================="
    print_success "依赖安装完成（含 WCDB）！"
    echo "========================================="
    echo ""
    print_info "所有依赖已安装到: $DEPENDENCIES_DIR/vcpkg_installed/"
    echo ""
}

main "$@"
