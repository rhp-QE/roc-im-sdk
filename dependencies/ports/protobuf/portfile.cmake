vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

set(PROTOBUF_VERSION "3.5.1")

vcpkg_download_distfile(ARCHIVE_FILE
    URLS "https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOBUF_VERSION}/protobuf-cpp-${PROTOBUF_VERSION}.tar.gz"
    FILENAME "protobuf-cpp-${PROTOBUF_VERSION}.tar.gz"
    SHA512 195ccb210229e0a1080dcdb0a1d87b2e421ad55f6b036c56db3183bd50a942c75b4cc84e6af8a10ad88022a247781a06f609a145a461dfbb8f04051b7dd714b3
)

vcpkg_extract_source_archive(
    SOURCE_PATH
    ARCHIVE "${ARCHIVE_FILE}"
)

# protobuf 3.5.1 contains old warning patterns on modern GCC/Clang.
vcpkg_configure_cmake(
    SOURCE_PATH "${SOURCE_PATH}/cmake"
    OPTIONS
        -Dprotobuf_BUILD_TESTS=OFF
        -Dprotobuf_BUILD_SHARED_LIBS=OFF
        -Dprotobuf_BUILD_EXAMPLES=OFF
        -Dprotobuf_WITH_ZLIB=ON
        -DCMAKE_CXX_STANDARD=14
        -DCMAKE_CXX_FLAGS=-Wno-error
)

vcpkg_install_cmake()

if(EXISTS "${CURRENT_PACKAGES_DIR}/bin/protoc")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/tools/protobuf")
    file(INSTALL "${CURRENT_PACKAGES_DIR}/bin/protoc" DESTINATION "${CURRENT_PACKAGES_DIR}/tools/protobuf")
endif()

if(EXISTS "${CURRENT_PACKAGES_DIR}/debug/bin/protoc")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/tools/protobuf")
    file(INSTALL "${CURRENT_PACKAGES_DIR}/debug/bin/protoc" DESTINATION "${CURRENT_PACKAGES_DIR}/tools/protobuf")
endif()

if(EXISTS "${CURRENT_PACKAGES_DIR}/share/protobuf/protobuf-targets-release.cmake")
    file(READ "${CURRENT_PACKAGES_DIR}/share/protobuf/protobuf-targets-release.cmake" RELEASE_MODULE)
    string(REPLACE "\${_IMPORT_PREFIX}/bin/protoc" "\${_IMPORT_PREFIX}/tools/protobuf/protoc" RELEASE_MODULE "${RELEASE_MODULE}")
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/protobuf/protobuf-targets-release.cmake" "${RELEASE_MODULE}")
endif()

if(EXISTS "${CURRENT_PACKAGES_DIR}/debug/share/protobuf/protobuf-targets-debug.cmake")
    file(READ "${CURRENT_PACKAGES_DIR}/debug/share/protobuf/protobuf-targets-debug.cmake" DEBUG_MODULE)
    string(REPLACE "\${_IMPORT_PREFIX}" "\${_IMPORT_PREFIX}/debug" DEBUG_MODULE "${DEBUG_MODULE}")
    string(REPLACE "\${_IMPORT_PREFIX}/debug/bin/protoc" "\${_IMPORT_PREFIX}/tools/protobuf/protoc" DEBUG_MODULE "${DEBUG_MODULE}")
    file(WRITE "${CURRENT_PACKAGES_DIR}/share/protobuf/protobuf-targets-debug.cmake" "${DEBUG_MODULE}")
endif()

# Keep debug/include for imported protobuf targets.
if(EXISTS "${CURRENT_PACKAGES_DIR}/include")
    file(MAKE_DIRECTORY "${CURRENT_PACKAGES_DIR}/debug")
    file(COPY "${CURRENT_PACKAGES_DIR}/include" DESTINATION "${CURRENT_PACKAGES_DIR}/debug")
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/bin")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/bin")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/protobuf" RENAME copyright)
