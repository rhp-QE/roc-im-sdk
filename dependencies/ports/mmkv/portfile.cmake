# MMKV vcpkg portfile for Linux/POSIX

vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Tencent/MMKV
    REF v1.3.9
    SHA512 b0c8b24857015b372fad67f4126fe997e9aff32d8054428138e4cef5e85de2701c181e0254737baa5e96f37c49318a30e1ec163d820cc1b90f705010ec3bc5c5
    HEAD_REF master
)

# Linux/POSIX 使用 POSIX/src 目录
set(MMKV_SOURCE_DIR "${SOURCE_PATH}/POSIX/src")

# 配置 CMake
vcpkg_cmake_configure(
    SOURCE_PATH "${MMKV_SOURCE_DIR}"
    OPTIONS
        -DMMKV_BUILD_SHARED_LIBS=OFF
)

# 构建
vcpkg_cmake_build()

# 手动安装（MMKV 没有定义 install 目标）
# 复制库文件（MMKV 构建生成 libcore.a）
file(INSTALL "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/Core/libcore.a"
     DESTINATION "${CURRENT_PACKAGES_DIR}/lib"
     RENAME libmmkv.a)
file(INSTALL "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/Core/libcore.a"
     DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib"
     RENAME libmmkv.a)

# 复制头文件到 include/MMKV/ 子目录（匹配用户代码的 #include "MMKV/MMKV.h"）
file(GLOB MMKV_HEADERS "${SOURCE_PATH}/Core/*.h" "${SOURCE_PATH}/Core/*.hpp")
file(INSTALL ${MMKV_HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include/MMKV")

# 创建 CMake 配置文件
file(WRITE "${CURRENT_PACKAGES_DIR}/share/mmkv/mmkvConfig.cmake" "
get_filename_component(_IMPORT_PREFIX \"\${CMAKE_CURRENT_LIST_FILE}\" PATH)
get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)
get_filename_component(_IMPORT_PREFIX \"\${_IMPORT_PREFIX}\" PATH)

include(CMakeFindDependencyMacro)
if(NOT TARGET mmkv::mmkv)
    add_library(mmkv::mmkv STATIC IMPORTED)
    set_target_properties(mmkv::mmkv PROPERTIES
        IMPORTED_LOCATION_RELEASE \"\${_IMPORT_PREFIX}/lib/libmmkv.a\"
        IMPORTED_LOCATION_DEBUG \"\${_IMPORT_PREFIX}/debug/lib/libmmkv.a\"
        INTERFACE_INCLUDE_DIRECTORIES \"\${_IMPORT_PREFIX}/include\"
    )
endif()
")

# 清理
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

# 安装版权文件
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.TXT")

# 复制使用说明
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
