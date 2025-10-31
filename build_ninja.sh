#!/bin/bash

# ROCIM 项目 Ninja 构建脚本
# 使用 Ninja 构建系统，显著提高编译速度

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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
    echo "  -j, --jobs N   指定并行任务数（默认: CPU核心数/5，最少2）"
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

# 计算合理的并行任务数（1/6 CPU 核心，向下取整，最少 2）
get_reasonable_jobs() {
    local total_cores=$(get_cpu_cores)
    local jobs=$((total_cores / 6))
    
    # 确保至少 2 核
    if [ $jobs -lt 2 ]; then
        jobs=2
    fi
    
    echo $jobs
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
    
    # 检查 vcpkg 工具链文件
    local vcpkg_root="${VCPKG_ROOT:-/opt/vcpkg}"
    local vcpkg_toolchain="${vcpkg_root}/scripts/buildsystems/vcpkg.cmake"
    
    if [ ! -f "$vcpkg_toolchain" ]; then
        print_error "vcpkg 工具链文件不存在: $vcpkg_toolchain"
        print_info "请设置 VCPKG_ROOT 环境变量或安装 vcpkg 到 /opt/vcpkg"
        exit 1
    fi
    
    print_info "vcpkg 工具链: $vcpkg_toolchain"
    
    # 在上级目录创建 build 文件夹
    if [ ! -d "../build" ]; then
        mkdir -p ../build
        print_info "创建构建目录: ../build"
    fi
    
    cd ../build
    
    # 使用 gcc-14/g++-14 直接配置并生成 compile_commands.json
    print_info "使用 gcc-14/g++-14 配置项目..."
    print_info "同时生成 compile_commands.json 供 clangd 使用"
    
    cmake -G Ninja \
          -DCMAKE_BUILD_TYPE=${build_type} \
          -DCMAKE_C_COMPILER=gcc-14 \
          -DCMAKE_CXX_COMPILER=g++-14 \
          -DCMAKE_TOOLCHAIN_FILE="$vcpkg_toolchain" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          ../roc-im-sdk
    
    if [ $? -eq 0 ]; then
        print_success "CMake 配置成功！"
        
        # 验证 build.ninja 文件是否生成
        if [ -f "build.ninja" ]; then
            print_success "✅ build.ninja 文件已生成"
        else
            print_error "❌ build.ninja 文件未生成"
            exit 1
        fi
        
        # 检查并复制 compile_commands.json
        if [ -f "compile_commands.json" ]; then
            print_success "✅ compile_commands.json 文件已生成"
            
            # 复制到上级目录供 clangd 使用
            cp compile_commands.json ../
            print_success "✅ compile_commands.json 已复制到项目根目录"
        else
            print_warning "⚠️  compile_commands.json 文件未生成"
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
        ninja -j$jobs -v
    elif [ "$silent_flag" = true ]; then
        # 完全静默：只显示错误和失败
        ninja -j$jobs -s
    else
        # 默认静默：只显示当前编译项和错误信息
        local build_success=true
        local error_detected=false
        local error_context_lines=0
        local context_buffer=()
        local buffer_size=5   # 保存最近5行作为错误前上下文
        
        # 直接运行 ninja 并实时处理输出
        while IFS= read -r line; do
            # 维护上下文缓冲区（不包含错误行）
            if ! echo "$line" | grep -q "error: \|Error: \|ERROR: \|ninja: error\|collect2\|ld returned\|undefined reference\|multiple definition\|undefined reference to"; then
                context_buffer+=("$line")
                if [ ${#context_buffer[@]} -gt $buffer_size ]; then
                    context_buffer=("${context_buffer[@]:1}")  # 移除最旧的行
                fi
            fi
            
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
            elif echo "$line" | grep -q "Building CXX object\|Building C object\|Scanning.*for.*dependencies"; then
                # 显示当前正在编译的文件
                echo "$line"
            elif echo "$line" | grep -q "error: \|Error: \|ERROR: \|ninja: error\|collect2\|ld returned\|undefined reference\|multiple definition\|undefined reference to"; then
                # 检测到错误，先显示错误前的上下文
                if [ ${#context_buffer[@]} -gt 0 ]; then
                    echo ""
                    echo -e "${YELLOW}🔍 错误前上下文 (5行)${NC}"
                    for ((i=0; i<${#context_buffer[@]}; i++)); do
                        echo "  ${context_buffer[$i]}"
                    done
                fi
                
                # 显示错误信息，高亮错误关键词
                echo ""
                echo -e "${RED}❌ 错误信息${NC}"
                echo "$line" | sed 's/\(error: \|Error: \|ERROR: \|undefined reference to\)/\x1b[1;31m\1\x1b[0m/g'
                build_success=false
                error_detected=true
                error_context_lines=10  # 显示错误后的10行作为上下文
                echo ""
                echo -e "${CYAN}📋 错误后上下文 (10行)${NC}"
                
                # 清空上下文缓冲区，为下一个错误做准备
                context_buffer=()
            elif echo "$line" | grep -q "FAILED"; then
                # 检测到 FAILED，显示上下文
                if [ ${#context_buffer[@]} -gt 0 ]; then
                    echo ""
                    echo -e "${BLUE}⚠️  FAILED 前上下文 (5行)${NC}"
                    for ((i=0; i<${#context_buffer[@]}; i++)); do
                        echo "  ${context_buffer[$i]}"
                    done
                fi
                
                # 显示 FAILED 信息，用淡色标记
                echo ""
                echo -e "${BLUE}⚠️  FAILED 信息${NC}"
                echo -e "${BLUE}$line${NC}"
                
                # 设置 FAILED 状态
                build_success=false
                error_detected=true
                error_context_lines=10  # 显示 FAILED 后的10行作为上下文
                echo ""
                echo -e "${BLUE}⚠️  FAILED 后上下文 (10行)${NC}"
                
                # 清空上下文缓冲区，为下一个错误做准备
                context_buffer=()
            elif [ "$error_detected" = true ] && [ $error_context_lines -gt 0 ]; then
                # 显示错误后的几行作为上下文
                echo "  $line"
                error_context_lines=$((error_context_lines - 1))
                # 当错误后上下文显示完毕时，添加分隔线并重置状态
                if [ $error_context_lines -eq 0 ]; then
                    echo ""
                    echo -e "\033[1;35m═══════════════════════════════════════════════════════════════\033[0m"
                    echo ""
                    # 重置错误状态，为下一个错误做准备
                    error_detected=false
                fi
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
    
    if [ $? -eq 0 ]; then
        print_success "构建成功！"
        local exe_size=$(ls -lh main | awk '{print $5}')
        print_info "可执行文件: $(pwd)/main"
        print_info "文件大小: $exe_size"
    else
        print_error "构建失败！"
        exit 1
    fi
    
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
    # 默认使用 1/6 CPU 核心（向下取整，最少 2），避免吃光所有计算资源
    local jobs=${NINJA_JOBS:-$(get_reasonable_jobs)}
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
            # 快捷命令
            build)
                shift
                ;;
            clean)
                clean_flag=true
                shift
                ;;
            run)
                run_flag=true
                shift
                ;;
            rebuild)
                clean_flag=true
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
