cmake_minimum_required(VERSION 3.28.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 编译器设置
find_program(CMAKE_C_COMPILER gcc-14)
find_program(CMAKE_CXX_COMPILER g++-14)

# 编译命令导出
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS_OUTPUT_PATH "../")

# vcpkg 配置
set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")
# 强制使用项目根目录的 vcpkg_installed（可选）
set(VCPKG_INSTALLED_DIR "${CMAKE_SOURCE_DIR}/vcpkg_installed")

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

# OPENSSL 库
# --------------------------------------------
find_package(OpenSSL REQUIRED)

message(STATUS "✅ Found OpenSSL libraries:")
message(STATUS "  - OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "  - OpenSSL include: ${OPENSSL_INCLUDE_DIR}")

target_link_libraries(main PRIVATE 
    OpenSSL::SSL
    OpenSSL::Crypto
)
# --------------------------------------------

# PROTOBUF 库
# --------------------------------------------
find_package(protobuf CONFIG REQUIRED)

# 验证 protobuf 版本
if(protobuf_VERSION)
    message(STATUS "✅ Found protobuf version: ${protobuf_VERSION}")
    # 确保版本符合要求（3.5.x，不依赖 Abseil）
    if(protobuf_VERSION VERSION_GREATER_EQUAL "3.6.0")
        message(WARNING "⚠️  protobuf version ${protobuf_VERSION} may depend on Abseil")
    else()
        message(STATUS "✅ protobuf version ${protobuf_VERSION} does not depend on Abseil")
    endif()
else()
    message(WARNING "⚠️  Could not determine protobuf version")
endif()

target_link_libraries(main PRIVATE 
    protobuf::libprotobuf 
    protobuf::libprotobuf-lite
)
# --------------------------------------------

# WCDB 库
# --------------------------------------------
# WCDB 路径配置
set(WCDB_ROOT "/root/ccdep/src/wcdb/src")
set(WCDB_BUILD_DIR "${WCDB_ROOT}/build")
set(WCDB_INCLUDE_DIR "${WCDB_BUILD_DIR}/export_headers")
set(WCDB_LIB_DIR "${WCDB_BUILD_DIR}")

# 检查 WCDB 库文件是否存在
if(EXISTS "${WCDB_LIB_DIR}/libWCDB.so")
    message(STATUS "✅ Found WCDB library: ${WCDB_LIB_DIR}/libWCDB.so")
else()
    message(FATAL_ERROR "❌ WCDB library not found at ${WCDB_LIB_DIR}/libWCDB.so")
endif()

# 检查 WCDB 头文件目录是否存在
if(EXISTS "${WCDB_INCLUDE_DIR}")
    message(STATUS "✅ Found WCDB include directory: ${WCDB_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "❌ WCDB include directory not found at ${WCDB_INCLUDE_DIR}")
endif()

# 添加 WCDB 头文件路径
target_include_directories(main PRIVATE ${WCDB_INCLUDE_DIR})

# 添加 WCDB 库文件路径
target_link_directories(main PRIVATE ${WCDB_LIB_DIR})

# 链接 WCDB 库
target_link_libraries(main PRIVATE 
    WCDB
    sqlcipher
    zstd
)

message(STATUS "✅ Found WCDB libraries:")
message(STATUS "  - WCDB: ${WCDB_LIB_DIR}/libWCDB.so")
message(STATUS "  - SQLCipher: ${WCDB_LIB_DIR}/libsqlcipher.a")
message(STATUS "  - Zstd: ${WCDB_LIB_DIR}/libzstd.a")
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

# 测试配置
include(CTest)
enable_testing()

# 项目信息
message(STATUS "🚀 Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "🚀 C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "🚀 Source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "🚀 Binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "🚀 vcpkg installed directory: ${VCPKG_INSTALLED_DIR}")

