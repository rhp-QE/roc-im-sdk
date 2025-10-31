cmake_minimum_required(VERSION 3.28.0)
project(rocim-sdk VERSION 0.1.0 LANGUAGES C CXX)

# C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "")
message(STATUS "════════════════════════════════════════")
message(STATUS "  编译 ROCIM SDK 静态库")
message(STATUS "════════════════════════════════════════")

# 收集 imsdk 源文件
file(GLOB_RECURSE IMSDK_SOURCES 
    "src/*.cpp"
    "src/*.cc"
)

# 收集 base 依赖源文件
file(GLOB_RECURSE BASE_SOURCES 
    "base/*.cpp"
    "base/*.cc"
)

message(STATUS "📦 收集源文件:")
message(STATUS "   - imsdk/src/  (SDK 核心)")
message(STATUS "   - imsdk/base/ (SDK 基础库)")

# 创建静态库
add_library(rocim-sdk STATIC ${IMSDK_SOURCES} ${BASE_SOURCES})

# 设置头文件路径
target_include_directories(rocim-sdk 
    PUBLIC 
        # 对外暴露 - SDK 核心接口 (rocimsdk/core/)
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include/rocimsdk/core>
        
        # 对外暴露 - SDK 基础库 (rocimsdk/base/)
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/base/include>
        $<INSTALL_INTERFACE:include/rocimsdk/base>
    PRIVATE
        # 内部使用
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# 添加第三方库头文件路径（仅编译需要，不链接）
# WCDB
if(DEFINED VCPKG_INSTALLED_DIR)
    if(WIN32)
        set(WCDB_ROOT "${VCPKG_INSTALLED_DIR}/x64-windows")
    else()
        set(WCDB_ROOT "${VCPKG_INSTALLED_DIR}/x64-linux")
    endif()
    
    target_include_directories(rocim-sdk SYSTEM PRIVATE ${WCDB_ROOT}/include)
    message(STATUS "   - WCDB 头文件: ${WCDB_ROOT}/include")
endif()

# vcpkg 包含路径
if(DEFINED VCPKG_INSTALLED_DIR)
    if(WIN32)
        set(VCPKG_INCLUDE "${VCPKG_INSTALLED_DIR}/x64-windows/include")
    else()
        set(VCPKG_INCLUDE "${VCPKG_INSTALLED_DIR}/x64-linux/include")
    endif()
    
    target_include_directories(rocim-sdk SYSTEM PRIVATE ${VCPKG_INCLUDE})
    message(STATUS "   - vcpkg 头文件: ${VCPKG_INCLUDE}")
endif()

# 注意：SDK 静态库不链接任何第三方库
# 链接操作由壳工程（或第三方用户）完成

# MSVC 编译选项
if(MSVC)
    target_compile_options(rocim-sdk PUBLIC /bigobj /utf-8)
    target_compile_definitions(rocim-sdk PUBLIC
        _WIN32_WINNT=0x0A00
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

message(STATUS "✅ ROCIM SDK 静态库配置完成")
message(STATUS "   输出: librocim-sdk.a")
message(STATUS "")
message(STATUS "📤 对外暴露头文件:")
message(STATUS "   - rocimsdk/core/  -> imsdk/src/include/")
message(STATUS "   - rocimsdk/base/  -> imsdk/base/include/")
message(STATUS "════════════════════════════════════════")
message(STATUS "")

# ============================================================================
# 编译时导出头文件到 build/export_headers（模拟安装后的目录结构）
# ============================================================================

set(EXPORT_HEADERS_DIR "${CMAKE_BINARY_DIR}/export_headers")

# 创建导出目录
file(MAKE_DIRECTORY ${EXPORT_HEADERS_DIR}/rocimsdk/core)
file(MAKE_DIRECTORY ${EXPORT_HEADERS_DIR}/rocimsdk/base)

# 复制核心头文件到导出目录
file(GLOB_RECURSE CORE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/include/*.hpp")
foreach(HEADER ${CORE_HEADERS})
    # 计算相对路径
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/include" ${HEADER})
    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
    
    # 创建目标目录
    if(REL_DIR)
        file(MAKE_DIRECTORY ${EXPORT_HEADERS_DIR}/rocimsdk/core/${REL_DIR})
    endif()
    
    # 复制文件
    configure_file(${HEADER} ${EXPORT_HEADERS_DIR}/rocimsdk/core/${REL_PATH} COPYONLY)
endforeach()

# 复制基础库头文件到导出目录
file(GLOB_RECURSE BASE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/base/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/base/include/*.hpp")
foreach(HEADER ${BASE_HEADERS})
    # 计算相对路径
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/base/include" ${HEADER})
    get_filename_component(REL_DIR ${REL_PATH} DIRECTORY)
    
    # 创建目标目录
    if(REL_DIR)
        file(MAKE_DIRECTORY ${EXPORT_HEADERS_DIR}/rocimsdk/base/${REL_DIR})
    endif()
    
    # 复制文件
    configure_file(${HEADER} ${EXPORT_HEADERS_DIR}/rocimsdk/base/${REL_PATH} COPYONLY)
endforeach()

message(STATUS "📁 头文件已导出到构建目录:")
message(STATUS "   ${EXPORT_HEADERS_DIR}/")
message(STATUS "   ├── rocimsdk/")
message(STATUS "   │   ├── core/     (SDK 核心接口)")
message(STATUS "   │   └── base/     (SDK 基础库)")
message(STATUS "════════════════════════════════════════")
message(STATUS "")

# ============================================================================
# 安装规则（可选）- 用于 install 命令
# ============================================================================

# 安装静态库
install(TARGETS rocim-sdk
    EXPORT rocim-sdk-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装核心头文件 (imsdk/src/include -> rocimsdk/core)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/include/
    DESTINATION include/rocimsdk/core
    FILES_MATCHING 
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# 安装基础库头文件 (imsdk/base/include -> rocimsdk/base)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/base/include/
    DESTINATION include/rocimsdk/base
    FILES_MATCHING 
    PATTERN "*.h"
    PATTERN "*.hpp"
)

# 安装 CMake 配置文件
install(EXPORT rocim-sdk-targets
    FILE rocim-sdk-targets.cmake
    NAMESPACE rocimsdk::
    DESTINATION lib/cmake/rocim-sdk
)

message(STATUS "📦 安装规则已配置:")
message(STATUS "   - 静态库: lib/librocim-sdk.a")
message(STATUS "   - 核心头文件: include/rocimsdk/core/")
message(STATUS "   - 基础头文件: include/rocimsdk/base/")
message(STATUS "════════════════════════════════════════")
message(STATUS "")

