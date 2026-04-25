#!/bin/bash

# 构建并安装 WCDB 到 dependencies/vcpkg_installed/x64-linux
# 说明：WCDB 官方暂无 vcpkg port，这里采用源码编译 + 项目内安装前缀

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPENDENCIES_DIR="$PROJECT_ROOT/dependencies"
INSTALL_PREFIX="$DEPENDENCIES_DIR/vcpkg_installed/x64-linux"
RELEASE_LIB_DIR="$INSTALL_PREFIX/lib"
DEBUG_LIB_DIR="$INSTALL_PREFIX/debug/lib"
SRC_ROOT="$DEPENDENCIES_DIR/_src"
BUILD_ROOT="$DEPENDENCIES_DIR/_build"
WCDB_SRC="$SRC_ROOT/wcdb"
WCDB_BUILD="$BUILD_ROOT/wcdb-linux"
# 默认锁定到上游最新稳定正式版（非 prerelease），避免跟随 master 引入新问题
DEFAULT_WCDB_REF="v2.1.15"
WCDB_REF="${WCDB_REF:-}"
if [ -z "$WCDB_REF" ] && [ -n "${WCDB_TAG:-}" ]; then
    WCDB_REF="$WCDB_TAG"
fi
if [ -z "$WCDB_REF" ]; then
    WCDB_REF="$DEFAULT_WCDB_REF"
fi
WCDB_C_COMPILER=""
WCDB_CXX_COMPILER=""

log_info() {
    echo "[INFO] $1"
}

log_error() {
    echo "[ERROR] $1" >&2
}

require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        log_error "缺少命令: $1"
        exit 1
    fi
}

prepare_repo() {
    mkdir -p "$SRC_ROOT" "$BUILD_ROOT"

    if [ ! -d "$WCDB_SRC/.git" ]; then
        log_info "克隆 WCDB 源码到 $WCDB_SRC ..."
        git clone https://github.com/Tencent/wcdb.git "$WCDB_SRC"
    else
        log_info "复用已存在的 WCDB 源码目录: $WCDB_SRC"
    fi

    (
        cd "$WCDB_SRC"
        if git rev-parse --verify "${WCDB_REF}^{commit}" >/dev/null 2>&1; then
            log_info "本地已存在 WCDB 版本: $WCDB_REF，跳过远程拉取"
        else
            log_info "本地未找到 $WCDB_REF，尝试从远程拉取..."
            git fetch --tags origin
            git fetch --depth 1 origin "$WCDB_REF"
        fi
        git checkout --detach "$WCDB_REF"
    )

    # WCDB 构建需要 sqlcipher 与 zstd 子模块；openssl 子模块较大且 Linux 构建不强依赖
    (cd "$WCDB_SRC" && git submodule update --init --depth 1 sqlcipher zstd)
}

build_wcdb() {
    # 优先与主工程保持一致，避免 LTO 版本不匹配（如 gcc-13 vs g++-14）
    if command -v gcc-14 >/dev/null 2>&1 && command -v g++-14 >/dev/null 2>&1; then
        WCDB_C_COMPILER="$(command -v gcc-14)"
        WCDB_CXX_COMPILER="$(command -v g++-14)"
        log_info "使用 GCC 工具链: $WCDB_C_COMPILER / $WCDB_CXX_COMPILER"
    else
        WCDB_C_COMPILER="$(command -v cc)"
        WCDB_CXX_COMPILER="$(command -v c++)"
        log_info "未找到 gcc-14/g++-14，使用默认编译器: $WCDB_C_COMPILER / $WCDB_CXX_COMPILER"
    fi

    # 避免复用旧编译器缓存，导致继续产出旧版本 LTO 字节码
    rm -rf "$WCDB_BUILD"

    log_info "开始配置 WCDB (Release, static)..."
    cmake -S "$WCDB_SRC/src" -B "$WCDB_BUILD" \
        -DCMAKE_C_COMPILER="$WCDB_C_COMPILER" \
        -DCMAKE_CXX_COMPILER="$WCDB_CXX_COMPILER" \
        -DCMAKE_C_STANDARD=11 \
        -DCMAKE_C_FLAGS="-D_GNU_SOURCE -Wno-error=implicit-function-declaration -DEVP_CIPHER_nid=EVP_CIPHER_get_nid -DEVP_CIPHER_key_length=EVP_CIPHER_get_key_length -DEVP_CIPHER_iv_length=EVP_CIPHER_get_iv_length -DEVP_CIPHER_block_size=EVP_CIPHER_get_block_size -DEVP_MD_size=EVP_MD_get_size" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DWCDB_CPP=ON \
        -DWCDB_ZSTD=ON \
        -DSKIP_WCONAN=ON

    log_info "开始编译 WCDB ..."
    cmake --build "$WCDB_BUILD" -j"$(nproc)"
}

install_wcdb() {
    local wcdb_lib=""
    local sqlcipher_lib="$WCDB_BUILD/libsqlcipher.a"
    local zstd_lib="$WCDB_BUILD/libzstd.a"
    mkdir -p "$RELEASE_LIB_DIR" "$DEBUG_LIB_DIR" "$INSTALL_PREFIX/include"

    if [ -f "$WCDB_BUILD/libWCDB.a" ]; then
        wcdb_lib="$WCDB_BUILD/libWCDB.a"
    elif [ -f "$WCDB_BUILD/libWCDB.so" ]; then
        wcdb_lib="$WCDB_BUILD/libWCDB.so"
    else
        log_error "未找到 WCDB 产物（期望 libWCDB.a 或 libWCDB.so）"
        exit 1
    fi

    if [ ! -d "$WCDB_BUILD/export_headers/WCDB" ]; then
        log_error "未找到 WCDB 头文件导出目录: $WCDB_BUILD/export_headers/WCDB"
        exit 1
    fi

    if [ ! -f "$sqlcipher_lib" ]; then
        log_error "未找到 sqlcipher 库: $sqlcipher_lib"
        exit 1
    fi

    if [ ! -f "$zstd_lib" ]; then
        log_error "未找到 zstd 库: $zstd_lib"
        exit 1
    fi

    log_info "安装 WCDB/sqlcipher/zstd 到 $RELEASE_LIB_DIR 与 $DEBUG_LIB_DIR ..."
    cp -f "$wcdb_lib" "$RELEASE_LIB_DIR/"
    cp -f "$wcdb_lib" "$DEBUG_LIB_DIR/"
    cp -f "$sqlcipher_lib" "$RELEASE_LIB_DIR/"
    cp -f "$sqlcipher_lib" "$DEBUG_LIB_DIR/"
    cp -f "$zstd_lib" "$RELEASE_LIB_DIR/"
    cp -f "$zstd_lib" "$DEBUG_LIB_DIR/"

    log_info "安装 WCDB 头文件到 $INSTALL_PREFIX/include/WCDB ..."
    rm -rf "$INSTALL_PREFIX/include/WCDB"
    cp -r "$WCDB_BUILD/export_headers/WCDB" "$INSTALL_PREFIX/include/"
}

main() {
    require_cmd git
    require_cmd cmake
    require_cmd nproc

    log_info "项目目录: $PROJECT_ROOT"
    log_info "依赖目录: $DEPENDENCIES_DIR"
    log_info "安装前缀: $INSTALL_PREFIX"
    log_info "WCDB 版本(固定): $WCDB_REF"

    prepare_repo
    build_wcdb
    install_wcdb

    log_info "WCDB 安装完成。"
    log_info "已安装库(Release): $RELEASE_LIB_DIR/libWCDB.* $RELEASE_LIB_DIR/libsqlcipher.a $RELEASE_LIB_DIR/libzstd.a"
    log_info "已安装库(Debug):   $DEBUG_LIB_DIR/libWCDB.* $DEBUG_LIB_DIR/libsqlcipher.a $DEBUG_LIB_DIR/libzstd.a"
    log_info "已安装头文件: $INSTALL_PREFIX/include/WCDB/"
}

main "$@"
