#!/bin/bash

# 脚本：使用 protoc 将 sdkws.proto 转换为 C++ 代码并移动到 proto/ 目录
# 作者：AI Assistant
# 日期：$(date)

set -e  # 遇到错误时退出

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

# 检查 protoc 是否安装
check_protoc() {
    if ! command -v protoc &> /dev/null; then
        print_error "protoc 未安装，请先安装 Protocol Buffers 编译器"
        print_info "安装方法："
        print_info "  Ubuntu/Debian: sudo apt-get install protobuf-compiler"
        print_info "  CentOS/RHEL: sudo yum install protobuf-compiler"
        print_info "  macOS: brew install protobuf"
        print_info "  或者从 https://github.com/protocolbuffers/protobuf/releases 下载"
        exit 1
    fi
    
    PROTOC_VERSION=$(protoc --version)
    print_info "检测到 protoc: $PROTOC_VERSION"
}

# 检查源文件
check_source_files() {
    if [ ! -f "sdkws.proto" ]; then
        print_error "源文件 sdkws.proto 不存在"
        exit 1
    fi
    
    print_info "源文件 sdkws.proto 存在"
}

# 创建目标目录
create_target_dirs() {
    PROTO_DIR="imsdk/src/core/network/proto"
    
    if [ ! -d "$PROTO_DIR" ]; then
        print_info "创建目标目录: $PROTO_DIR"
        mkdir -p "$PROTO_DIR"
    else
        print_info "目标目录已存在: $PROTO_DIR"
    fi
}

# 备份现有文件
backup_existing_files() {
    PROTO_DIR="imsdk/src/core/network/proto"
    
    if [ -f "$PROTO_DIR/sdkws.pb.h" ] || [ -f "$PROTO_DIR/sdkws.pb.cc" ]; then
        BACKUP_DIR="${PROTO_DIR}/backup_$(date +%Y%m%d_%H%M%S)"
        print_info "备份现有文件到: $BACKUP_DIR"
        mkdir -p "$BACKUP_DIR"
        cp -f "$PROTO_DIR"/sdkws.pb.* "$BACKUP_DIR/" 2>/dev/null || true
        print_success "备份完成"
    fi
}

# 生成 C++ 代码
generate_cpp_code() {
    print_info "开始生成 C++ 代码..."
    
    # 使用 protoc 生成 C++ 代码
    protoc --cpp_out=. sdkws.proto
    
    if [ $? -eq 0 ]; then
        print_success "C++ 代码生成成功"
    else
        print_error "C++ 代码生成失败"
        exit 1
    fi
}

# 移动生成的文件
move_generated_files() {
    PROTO_DIR="imsdk/src/core/network/proto"
    
    print_info "移动生成的文件到目标目录..."
    
    # 移动头文件
    if [ -f "sdkws.pb.h" ]; then
        mv sdkws.pb.h "$PROTO_DIR/"
        print_success "移动 sdkws.pb.h 到 $PROTO_DIR/"
    else
        print_error "sdkws.pb.h 不存在"
        exit 1
    fi
    
    # 移动源文件
    if [ -f "sdkws.pb.cc" ]; then
        mv sdkws.pb.cc "$PROTO_DIR/"
        print_success "移动 sdkws.pb.cc 到 $PROTO_DIR/"
    else
        print_error "sdkws.pb.cc 不存在"
        exit 1
    fi
}

# 清理临时文件
cleanup() {
    print_info "清理临时文件..."
    
    # 删除可能存在的临时文件
    rm -f sdkws.pb.* 2>/dev/null || true
    
    print_success "清理完成"
}

# 验证生成的文件
verify_generated_files() {
    PROTO_DIR="imsdk/src/core/network/proto"
    
    print_info "验证生成的文件..."
    
    if [ -f "$PROTO_DIR/sdkws.pb.h" ] && [ -f "$PROTO_DIR/sdkws.pb.cc" ]; then
        print_success "文件验证成功"
        print_info "生成的文件："
        ls -la "$PROTO_DIR"/sdkws.pb.*
    else
        print_error "文件验证失败"
        exit 1
    fi
}

# 主函数
main() {
    print_info "开始执行 protobuf 代码生成脚本..."
    print_info "当前工作目录: $(pwd)"
    
    # 检查依赖
    check_protoc
    
    # 检查源文件
    check_source_files
    
    # 创建目标目录
    create_target_dirs
    
    # 备份现有文件
    backup_existing_files
    
    # 生成 C++ 代码
    generate_cpp_code
    
    # 移动生成的文件
    move_generated_files
    
    # 清理临时文件
    cleanup
    
    # 验证生成的文件
    verify_generated_files
    
    print_success "所有操作完成！"
    print_info "生成的 C++ 文件已移动到: imsdk/src/core/network/proto/"
}

# 执行主函数
main "$@"
