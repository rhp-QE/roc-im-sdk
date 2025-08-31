#!/bin/bash

# ROCIM 项目 Ninja 构建脚本
# 使用 Ninja 构建系统，显著提高编译速度

set -e  # 遇到错误立即退出

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

# 显示帮助信息
show_help() {
    echo "ROCIM 项目 Ninja 构建脚本"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help     显示此帮助信息"
    echo "  -c, --clean    清理构建文件后重新构建"
    echo "  -r, --run      构建后运行程序"
    echo "  -f, --full     完整流程（清理 + 构建 + 运行）"
    echo "  -j, --jobs N   指定并行任务数（默认使用所有核心）"
    echo "  -d, --debug    使用 Debug 模式构建"
    echo "  -R, --release  使用 Release 模式构建"
    echo "  --reconfigure  强制重新配置 CMake 项目"
    echo "  -v, --verbose  显示详细构建信息"
    echo "  -q, --quiet    静默模式（默认）"
    echo "  -s, --silent   完全静默（只显示错误）"
    echo ""
    echo "环境变量:"
echo "  NINJA_JOBS     设置 Ninja 并行任务数"
echo "  VERBOSE_BUILD  设置为 'true' 时显示详细构建信息"
    echo ""
    echo "示例:"
echo "  $0              # 普通构建（静默模式）"
echo "  $0 -c           # 清理后构建"
echo "  $0 -r           # 构建后运行"
echo "  $0 -f           # 完整流程"
echo "  $0 -j 8         # 使用8个并行任务"
echo "  $0 -d           # Debug 模式构建"
echo "  $0 -v           # 详细构建信息"
echo "  $0 -q           # 静默模式（默认，显示进度）"
echo "  $0 -s           # 完全静默（只显示错误）"
}

# 检查 Ninja 是否安装
check_ninja() {
    if ! command -v ninja &> /dev/null; then
        print_error "Ninja 未安装！请先安装 Ninja："
        echo "  Ubuntu/Debian: sudo apt install ninja-build"
        echo "  CentOS/RHEL: sudo yum install ninja-build"
        echo "  macOS: brew install ninja"
        exit 1
    fi
    
    print_info "✅ 找到 Ninja: $(ninja --version)"
}

# 获取CPU核心数
get_cpu_cores() {
    echo $(nproc)
}

# 清理构建文件
clean_build() {
    print_info "清理构建文件..."
    if [ -d "../build" ]; then
        cd ../build
        ninja clean
        cd ../roc-im-sdk
    else
        print_warning "../build 目录不存在，跳过清理"
    fi
    print_success "清理完成"
}

# 配置 CMake 项目
configure_project() {
    local build_type=$1
    print_info "配置 CMake 项目..."
    print_info "构建类型: ${build_type}"
    
    # 在上级目录创建 build 文件夹
    if [ ! -d "../build" ]; then
        mkdir -p ../build
        print_info "创建构建目录: ../build"
    fi
    
    cd ../build
    
    # 使用 Ninja 生成器配置，指向 roc-im-sdk 目录
    print_info "运行 CMake 配置..."
    cmake -G Ninja -DCMAKE_BUILD_TYPE=${build_type} ../roc-im-sdk
    
    if [ $? -eq 0 ]; then
        print_success "CMake 配置成功！"
        # 验证 build.ninja 文件是否生成
        if [ -f "build.ninja" ]; then
            print_success "✅ build.ninja 文件已生成"
        else
            print_error "❌ build.ninja 文件未生成"
            exit 1
        fi
    else
        print_error "CMake 配置失败！"
        exit 1
    fi
    
    cd ../roc-im-sdk
}

# 构建项目
build_project() {
    local jobs=$1
    print_info "开始构建项目..."
    print_info "并行任务数: $jobs"
    
    cd ../build
    
    # 使用 Ninja 构建（根据标志决定输出级别）
    if [ "$verbose_flag" = true ]; then
        # 详细模式：显示所有信息
        if ! ninja -j$jobs -v; then
            print_error "构建失败！"
            exit 1
        fi
    elif [ "$silent_flag" = true ]; then
        # 完全静默：只显示错误和失败
        local temp_output=$(mktemp)
        if ninja -j$jobs 2>&1 | tee "$temp_output"; then
            # 构建成功
            rm -f "$temp_output"
        else
            # 构建失败，显示错误信息
            echo "构建失败，错误详情："
            cat "$temp_output" | grep -E "(error|Error|ERROR|FAILED|ninja: error)" || true
            rm -f "$temp_output"
            print_error "构建失败！"
            exit 1
        fi
    else
        # 默认静默：显示高亮进度，隐藏警告
        local build_success=true
        local error_detected=false
        
        # 直接运行 ninja 并实时处理输出
        while IFS= read -r line; do
            if echo "$line" | grep -q "\[[0-9]*/[0-9]*\]"; then
                # 只高亮进度数字部分，保持文件名正常显示
                if [[ "$line" =~ \[([0-9]+)/([0-9]+)\] ]]; then
                    local progress="${BASH_REMATCH[0]}"
                    local before_progress="${line%\[*}"
                    local after_progress="${line#*\]}"
                    echo -e "${before_progress}${GREEN}${progress}${NC}${after_progress}"
                else
                    echo "$line"
                fi
            elif echo "$line" | grep -q "error\|Error\|ERROR\|FAILED\|ninja: error\|collect2\|ld returned\|undefined reference\|multiple definition"; then
                # 显示错误信息，包括链接错误，高亮错误关键词
                echo "$line" | sed 's/\(error\|Error\|ERROR\|FAILED\)/\x1b[1;31m\1\x1b[0m/g'
                build_success=false
                error_detected=true
            elif [ "$error_detected" = true ]; then
                # 如果已经检测到错误，显示后续的相关行，也高亮错误关键词
                echo "$line" | sed 's/\(error\|Error\|ERROR\|FAILED\)/\x1b[1;31m\1\x1b[0m/g'
            elif echo "$line" | grep -q "ninja: no work to do\|Linking CXX executable\|Built target"; then
                # 显示成功信息
                echo "$line"
            fi
        done < <(ninja -j$jobs 2>&1; echo "NINJA_EXIT_CODE:$?")
        
        # 检查 ninja 的退出状态
        if [ $? -ne 0 ] || ! $build_success; then
            print_error "构建失败！"
            exit 1
        fi
    fi
    
    # 构建成功，显示结果
    print_success "构建成功！"
    local exe_size=$(ls -lh main | awk '{print $5}')
    print_info "可执行文件: $(pwd)/main"
    print_info "文件大小: $exe_size"
    
    cd ../roc-im-sdk
}

# 运行程序
run_program() {
    print_info "运行程序..."
    cd ../build
    if [ -f main ]; then
        ./main
    else
        print_error "可执行文件不存在，请先构建"
        exit 1
    fi
    
    cd ../roc-im-sdk
}

# 主函数
main() {
    local clean_flag=false
    local run_flag=false
    local build_type="Debug"
    local jobs=${NINJA_JOBS:-$(get_cpu_cores)}
    local verbose_flag=false
    local silent_flag=false
    
    # 解析命令行参数
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_flag=true
                shift
                ;;
            -r|--run)
                run_flag=true
                shift
                ;;
            -f|--full)
                clean_flag=true
                run_flag=true
                shift
                ;;
            -j|--jobs)
                if [[ -n $2 ]] && [[ $2 =~ ^[0-9]+$ ]]; then
                    jobs=$2
                    shift 2
                else
                    print_error "请指定有效的任务数"
                    exit 1
                fi
                ;;
            -d|--debug)
                build_type="Debug"
                shift
                ;;
            -R|--release)
                build_type="Release"
                shift
                ;;
            --reconfigure)
                clean_flag=true
                shift
                ;;
            -v|--verbose)
                verbose_flag=true
                shift
                ;;
            -q|--quiet)
                verbose_flag=false
                silent_flag=false
                shift
                ;;
            -s|--silent)
                verbose_flag=false
                silent_flag=true
                shift
                ;;
            *)
                print_error "未知选项: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    echo "=== ROCIM 项目 Ninja 构建脚本 ==="
    
    # 检查 Ninja
    check_ninja
    
    # 执行清理
    if [ "$clean_flag" = true ]; then
        clean_build
    fi
    
    # 配置项目（如果 build 目录不存在或需要重新配置）
    if [ ! -d "../build" ] || [ "$clean_flag" = true ] || [ ! -f "../build/build.ninja" ]; then
        configure_project $build_type
    fi
    
    # 执行构建
    build_project $jobs
    
    # 执行运行
    if [ "$run_flag" = true ]; then
        run_program
    fi
    
    print_success "所有操作完成！"
    print_info "💡 提示：使用 'ninja -j$jobs' 可以更快地重新构建"
}

# 运行主函数
main "$@"
