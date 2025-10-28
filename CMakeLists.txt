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
    # 如果没有 Ninja，尝试使用 Makefiles 作为备选
    set(CMAKE_GENERATOR "Unix Makefiles")
endif()


# vcpkg 配置
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")
# 强制使用项目根目录的 vcpkg_installed（可选）
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/../vcpkg_installed")

# -------------------------------------
project(ROCIM_CLIENT VERSION 0.1.0 LANGUAGES C CXX)

# -------------------------------------

# 包含目录
include_directories(${CMAKE_SOURCE_DIR})

# 源文件收集
file(GLOB_RECURSE SOURCES 
  "im/*.cpp"
  "im/*.cc"
  "test/*.cpp"
  "test/*.cc"
  "base/*.cpp"
  "base/*.cc"
  "sdk/*.cpp"
  "sdk/*.cc"
  "imsdk/*.cpp"
  "imsdk/*.cc"
  "adapter/*.cpp"
  "adapter/*.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

# 创建可执行文件
add_executable(main ${SOURCES})

#--------------------------------------
# 依赖包配置
#--------------------------------------

# BOOST 库
# --------------------------------------------
find_package(boost_asio CONFIG REQUIRED)
find_package(boost_system CONFIG REQUIRED)
find_package(boost_json CONFIG REQUIRED)
find_package(boost_beast CONFIG REQUIRED)

message(STATUS "✅ Found Boost libraries:")
message(STATUS "  - boost_asio")
message(STATUS "  - boost_system") 
message(STATUS "  - boost_json")
message(STATUS "  - boost_beast")

target_link_libraries(main PRIVATE 
    Boost::beast
    Boost::asio
    Boost::system
    Boost::json
)
# --------------------------------------------

# MMKV 库 - 通过 vcpkg 自定义端口
# --------------------------------------------
find_package(mmkv CONFIG REQUIRED)
message(STATUS "✅ Found MMKV (via vcpkg 自定义端口)")
target_link_libraries(main PRIVATE mmkv::mmkv)
# --------------------------------------------

# OPENSSL 库
# --------------------------------------------
find_package(OpenSSL REQUIRED)
message(STATUS "✅ Found OpenSSL library:")
message(STATUS "  - OpenSSL version: ${OPENSSL_VERSION}")
target_link_libraries(main PRIVATE 
    OpenSSL::SSL
    OpenSSL::Crypto
)
# --------------------------------------------

# PROTOBUF 库
# --------------------------------------------
find_package(protobuf CONFIG REQUIRED)
message(STATUS "✅ Found protobuf version: ${protobuf_VERSION}")
target_link_libraries(main PRIVATE 
    protobuf::libprotobuf 
    protobuf::libprotobuf-lite
)
# --------------------------------------------

# SPDLOG 库
# --------------------------------------------
find_package(spdlog CONFIG REQUIRED)
message(STATUS "  - spdlog version: ${spdlog_VERSION}")
target_link_libraries(main PRIVATE 
    spdlog::spdlog
)
# --------------------------------------------

# WCDB 库 - 从 vcpkg_installed 目录查找
# --------------------------------------------
# WCDB 路径配置（vcpkg manifest 模式）
set(WCDB_ROOT "${VCPKG_INSTALLED_DIR}/x64-linux")
set(WCDB_INCLUDE_DIR "${WCDB_ROOT}/include")
set(WCDB_LIB_DIR "${WCDB_ROOT}/lib")

# 添加 WCDB 头文件（标记为 SYSTEM 以抑制警告）
target_include_directories(main SYSTEM PRIVATE ${WCDB_INCLUDE_DIR})
# 添加库路径
target_link_directories(main PRIVATE ${WCDB_LIB_DIR})
# 链接 WCDB 及其子模块（参考原始版本）
target_link_libraries(main PRIVATE 
    WCDB
    sqlcipher
    zstd
)

message(STATUS "✅ WCDB configured:")
message(STATUS "  - Library: ${WCDB_LIBRARY}")
message(STATUS "  - Headers: ${WCDB_INCLUDE_DIR}")
message(STATUS "  - sqlcipher: ${WCDB_LIB_DIR}/libsqlcipher.a")
message(STATUS "  - zstd: ${WCDB_LIB_DIR}/libzstd.a")
# --------------------------------------------

# 构建配置
# --------------------------------------------

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "🔧 Build type not specified, defaulting to Debug")
endif()

message(STATUS "🔧 Build type: ${CMAKE_BUILD_TYPE}")

# 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra")
    message(STATUS "🔧 Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    message(STATUS "🔧 Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# 链接器配置
# 使用平台默认链接器，确保与 vcpkg 库的 LTO 设置一致
message(STATUS "🔧 Using platform default linker for LTO compatibility")

# Ninja 构建优化
if(CMAKE_GENERATOR STREQUAL "Ninja")
    # 启用并行编译
    set(CMAKE_NINJA_PARALLEL_JOBS $ENV{NINJA_JOBS})
    if(NOT CMAKE_NINJA_PARALLEL_JOBS)
        # 如果没有设置环境变量，使用 CPU 核心数
        include(ProcessorCount)
        ProcessorCount(N)
        if(N GREATER 0)
            set(CMAKE_NINJA_PARALLEL_JOBS ${N})
        else()
            set(CMAKE_NINJA_PARALLEL_JOBS 4)  # 默认值
        endif()
    endif()
    
    message(STATUS "🚀 Ninja parallel jobs: ${CMAKE_NINJA_PARALLEL_JOBS}")
    
    # Ninja 特定的编译优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")  # 减少临时文件
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe")
    
    # 启用增量编译优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")
    
    # 设置链接器的并行作业数
    set(CMAKE_NINJA_PARALLEL_LINK_JOBS $ENV{NINJA_LINK_JOBS})
    if(NOT CMAKE_NINJA_PARALLEL_LINK_JOBS)
        # 链接器通常使用较少的并行度，避免内存不足
        if(N GREATER 2)
            set(CMAKE_NINJA_PARALLEL_LINK_JOBS 2)
        else()
            set(CMAKE_NINJA_PARALLEL_LINK_JOBS 1)
        endif()
    endif()
    
    message(STATUS "🚀 Ninja parallel link jobs: ${CMAKE_NINJA_PARALLEL_LINK_JOBS}")
endif()

# 测试配置
include(CTest)
enable_testing()

# 项目信息
message(STATUS "🚀 Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "🚀 C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "🚀 C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "🚀 C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "🚀 Build system: ${CMAKE_GENERATOR}")
if(CMAKE_GENERATOR STREQUAL "Ninja")
    message(STATUS "🚀 Ninja parallel jobs: ${CMAKE_NINJA_PARALLEL_JOBS}")
endif()
message(STATUS "🚀 Linker: Platform default linker")
message(STATUS "🚀 Source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "🚀 Binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "🚀 vcpkg installed directory: ${VCPKG_INSTALLED_DIR}")

