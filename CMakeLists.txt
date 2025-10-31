cmake_minimum_required(VERSION 3.28.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# # 强制设置编译器为 Clang
# set(CMAKE_C_COMPILER gcc-14)
# set(CMAKE_CXX_COMPILER g++-14)

# # 编译命令导出
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS_OUTPUT_PATH "../")

# set(CMAKE_C_COMPILER clang)
# set(CMAKE_CXX_COMPILER clang++)

# Ninja 构建系统配置
find_program(CMAKE_MAKE_PROGRAM ninja)
if(CMAKE_MAKE_PROGRAM)
    message(STATUS "✅ Found Ninja build system: ${CMAKE_MAKE_PROGRAM}")
    set(CMAKE_GENERATOR "Ninja")
else()
    message(STATUS "ℹ️  Ninja not found, using default generator")
    if(NOT WIN32)
        # 非 Windows 平台回退到 Unix Makefiles
        set(CMAKE_GENERATOR "Unix Makefiles")
    endif()
endif()


# vcpkg 配置
# 工具链文件应通过命令行传入：-DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg.cmake>
# 强制使用项目根目录的 vcpkg_installed（可选）
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/../vcpkg_installed")

# -------------------------------------
project(rocim-client VERSION 0.1.0 LANGUAGES C CXX)

# -------------------------------------

message(STATUS "")
message(STATUS "════════════════════════════════════════════════════════════")
message(STATUS "  ROCIM 项目构建")
message(STATUS "  1. 编译 imsdk -> librocim-sdk.a")
message(STATUS "  2. 编译壳工程 -> main 可执行文件")
message(STATUS "════════════════════════════════════════════════════════════")
message(STATUS "")

# ============================================================================
# 第一步：编译 imsdk 子项目（生成 librocim-sdk.a）
# ============================================================================
add_subdirectory(imsdk)

# 为壳工程创建别名
add_library(rocim::sdk ALIAS rocim-sdk)

# ============================================================================
# 第二步：编译壳工程（演示如何使用 SDK）
# ============================================================================

message(STATUS "════════════════════════════════════════")
message(STATUS "  编译壳工程（演示如何使用 SDK）")
message(STATUS "════════════════════════════════════════")

# 收集壳工程源文件
file(GLOB_RECURSE SHELL_SOURCES 
    "im/*.cpp"
    "im/*.cc"
    "adapter/*.cpp"
    "adapter/*.cc"
    "test/*.cpp"
    "test/*.cc"
    "examples/*.cpp"
    "examples/*.cc"
)

message(STATUS "📦 壳工程源文件:")
message(STATUS "   - main.cpp")
message(STATUS "   - im/")
message(STATUS "   - adapter/")
message(STATUS "   - test/")
message(STATUS "   - examples/")

# 创建可执行文件
add_executable(main main.cpp ${SHELL_SOURCES})

# 添加头文件路径
target_include_directories(main PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/imsdk/src
    ${CMAKE_SOURCE_DIR}/imsdk/base/include
)

# ============================================================================
# 演示：壳工程如何链接 SDK 和所有依赖（给第三方用户参考）
# ============================================================================

message(STATUS "")
message(STATUS "📚 查找并链接所有依赖库:")

# 1. 查找 Boost
find_package(Boost REQUIRED COMPONENTS system json)
message(STATUS "   ✅ Boost ${Boost_VERSION} (system, json, asio, beast)")

# 2. 查找 Protobuf
find_package(protobuf CONFIG REQUIRED)
message(STATUS "   ✅ Protobuf ${protobuf_VERSION}")

# 3. 查找 MMKV
find_package(mmkv CONFIG REQUIRED)
message(STATUS "   ✅ MMKV")

# 4. 查找 OpenSSL
find_package(OpenSSL REQUIRED)
message(STATUS "   ✅ OpenSSL ${OPENSSL_VERSION}")

# 5. 查找 spdlog
find_package(spdlog CONFIG REQUIRED)
message(STATUS "   ✅ spdlog ${spdlog_VERSION}")

# 6. 查找 WCDB
if(WIN32)
    set(WCDB_ROOT "${VCPKG_INSTALLED_DIR}/x64-windows")
else()
    set(WCDB_ROOT "${VCPKG_INSTALLED_DIR}/x64-linux")
endif()

find_library(WCDB_LIBRARY NAMES WCDB PATHS ${WCDB_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
find_library(SQLCIPHER_LIBRARY NAMES sqlcipher PATHS ${WCDB_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
find_library(ZSTD_LIBRARY NAMES zstd PATHS ${WCDB_ROOT}/lib NO_DEFAULT_PATH REQUIRED)

message(STATUS "   ✅ WCDB + sqlcipher + zstd")

# ============================================================================
# 链接顺序：SDK 库 + 所有第三方依赖（演示给用户的完整链接过程）
# ============================================================================

target_link_libraries(main PRIVATE 
    # 1. SDK 静态库（必须第一个）
    rocim::sdk
    
    # 2. Boost 库
    Boost::boost          # header-only (asio, beast)
    Boost::system         # 显式链接（可能会重复，链接器会自动去重）
    Boost::json           # 会自动拉 Boost::system
    
    # 3. Protobuf
    protobuf::libprotobuf
    protobuf::libprotobuf-lite
    
    # 4. MMKV
    mmkv::mmkv
    
    # 5. WCDB（数据库）
    ${WCDB_LIBRARY}
    ${SQLCIPHER_LIBRARY}
    ${ZSTD_LIBRARY}
    
    # 6. OpenSSL
    OpenSSL::SSL
    OpenSSL::Crypto
    
    # 7. spdlog（日志）
    spdlog::spdlog
    
    # 8. 系统库
    pthread
    dl
)

# 说明：
# - Boost::json 内部依赖 Boost::system，CMake 会自动添加
# - spdlog 也可能依赖 Boost::system
# - 导致 Boost::system 可能被链接 2 次
# - 这是正常的，链接器会自动去重，不会有问题

message(STATUS "")
message(STATUS "✅ 壳工程链接配置（演示给用户）:")
message(STATUS "   1. rocim::sdk          (librocim-sdk.a)")
message(STATUS "   2. Boost               (system, json, asio, beast)")
message(STATUS "   3. Protobuf            (3.5.1)")
message(STATUS "   4. MMKV                (键值存储)")
message(STATUS "   5. WCDB                (数据库 + sqlcipher + zstd)")
message(STATUS "   6. OpenSSL             (加密)")
message(STATUS "   7. spdlog              (日志)")
message(STATUS "   8. pthread, dl         (系统库)")
message(STATUS "════════════════════════════════════════")
message(STATUS "")

# 构建配置
# --------------------------------------------

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "🔧 Build type not specified, defaulting to Debug")
endif()

message(STATUS "🔧 Build type: ${CMAKE_BUILD_TYPE}")

# 编译选项（区分 MSVC 与 GCC/Clang）
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "🔧 Debug flags (MSVC): ${CMAKE_CXX_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "🔧 Release flags (MSVC): ${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra")
        message(STATUS "🔧 Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
        message(STATUS "🔧 Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
endif()

# 链接器配置
# 使用平台默认链接器，确保与 vcpkg 库的 LTO 设置一致
message(STATUS "🔧 Using platform default linker for LTO compatibility")

# Ninja 构建优化
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # 启用并行编译（限制为 CPU 核心数的 1/6，避免吃光 CPU）
    set(CMAKE_NINJA_PARALLEL_JOBS $ENV{NINJA_JOBS})
    if(NOT CMAKE_NINJA_PARALLEL_JOBS)
        # 计算合理的并行数：总核心数 / 6，向下取整，最少 2
        include(ProcessorCount)
        ProcessorCount(N)
        if(N GREATER 0)
            math(EXPR REASONABLE_JOBS "${N} / 6")
            if(REASONABLE_JOBS LESS 2)
                set(CMAKE_NINJA_PARALLEL_JOBS 2)
            else()
                set(CMAKE_NINJA_PARALLEL_JOBS ${REASONABLE_JOBS})
            endif()
        else()
            set(CMAKE_NINJA_PARALLEL_JOBS 2)
        endif()
    endif()
    
    message(STATUS "🚀 Ninja parallel jobs: ${CMAKE_NINJA_PARALLEL_JOBS} / ${N} 核 (限制 1/6 CPU，避免占用过多资源)")
    
    # Ninja 特定的编译优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")  # 减少临时文件
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe")
    
    # 启用增量编译优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    
    # 设置链接器的并行作业数（链接内存消耗大，使用 1 个）
    set(CMAKE_NINJA_PARALLEL_LINK_JOBS 1)
    
    message(STATUS "🚀 Ninja parallel link jobs: ${CMAKE_NINJA_PARALLEL_LINK_JOBS} (链接内存消耗大)")
endif()

# 测试配置
include(CTest)
enable_testing()

# MSVC VS 调试设置
if(MSVC)
    set_target_properties(main PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.."
    )
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
endif()

