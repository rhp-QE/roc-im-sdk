#pragma once

#include <MMKV/MMKV.h>
#include <iostream>

inline void test_mmkv() {
    // 使用相对路径（相对于运行时工作目录）
#ifdef _WIN32
    // Windows 上 MMKVPath_t 是 std::wstring
    std::wstring rootDir = L"./db_data";
    MMKV::initializeMMKV(rootDir);
#else
    // Linux/POSIX 上 MMKVPath_t 是 std::string
    std::string rootDir = "./db_data";
    MMKV::initializeMMKV(rootDir);
#endif

    auto mmkv = MMKV::defaultMMKV();
    mmkv->set(true, "bool");
    std::cout << "bool = " << mmkv->getBool("bool") << std::endl;

    mmkv->set(1024, "int32");
    std::cout << "int32 = " << mmkv->getInt32("int32") << std::endl;

    mmkv->set("Hello, MMKV for Windows", "string");
    std::string result;
    mmkv->getString("string", result);
    std::cout << "string = " << result << std::endl;

}