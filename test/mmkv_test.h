#pragma once

#include <MMKV/MMKV.h>
#include <iostream>

inline void test_mmkv() {
    std::string rootDir = "/root/project/roc_im_sdk/db-data";
    MMKV::initializeMMKV(rootDir);

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