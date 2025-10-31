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
    "../base/*.cpp"
    "../base/*.cc"
)

message(STATUS "📦 收集源文件:")
message(STATUS "   - imsdk/src/  (SDK 核心)")
message(STATUS "   - base/       (SDK 依赖)")

# 创建静态库
add_library(rocim-sdk STATIC ${IMSDK_SOURCES} ${BASE_SOURCES})

# 设置头文件路径
target_include_directories(rocim-sdk 
    PUBLIC 
        # 对外暴露
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # 内部使用
        ${CMAKE_CURRENT_SOURCE_DIR}/src
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
message(STATUS "════════════════════════════════════════")
message(STATUS "")

