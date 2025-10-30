#ifndef ROC_TEST_WCDB_TEST_H
#define ROC_TEST_WCDB_TEST_H

#include <iostream>
#include <string>
#include "base/database/WCDBExample.h"

// 测试 WCDB 库是否正确链接
inline void test_wcdb_linking() {
    std::cout << "Testing WCDB library linking..." << std::endl;
    
    try {
        // 创建 WCDB 管理器
        roc::base::database::WCDBManager db_manager;
        
        // 初始化数据库 - 使用相对路径
        bool ret = db_manager.init_database("./db_data/test_wcdb.db");
        if (ret) {
            std::cout << "✅ WCDB library linking test passed" << std::endl;
        } else {
            std::cout << "❌ WCDB library linking test failed" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "❌ WCDB library linking test failed: " << e.what() << std::endl;
    }
}

// 测试 WCDB 基本功能
inline void test_wcdb_basic_operations() {
    std::cout << "Testing WCDB basic operations..." << std::endl;
    
    try {
        roc::base::database::WCDBManager db_manager;
        
        // 初始化数据库 - 使用相对路径
        if (!db_manager.init_database("./db_data/test_operations.db")) {
            std::cout << "❌ Failed to initialize database" << std::endl;
            return;
        }
        
        // 创建表
        if (!db_manager.create_tables()) {
            std::cout << "❌ Failed to create tables" << std::endl;
            return;
        }
        
        // 插入消息
        roc::base::database::Message msg1(1, "Hello WCDB!");
        roc::base::database::Message msg2(2, "This is a test message");
        
        if (db_manager.insert_message(msg1) && db_manager.insert_message(msg2)) {
            std::cout << "✅ Message insertion test passed" << std::endl;
        } else {
            std::cout << "❌ Message insertion test failed" << std::endl;
        }
        
        // 查询消息
        auto messages = db_manager.get_all_messages();
        if (messages.hasValue() && messages->size() >= 2) {
            std::cout << "✅ Message query test passed, found " << messages->size() << " messages" << std::endl;
        } else {
            std::cout << "❌ Message query test failed" << std::endl;
        }
        
        // 插入会话
        roc::base::database::Conversation conv1("conv_001", "Test Conversation", 1234567890);
        if (db_manager.insert_conversation(conv1)) {
            std::cout << "✅ Conversation insertion test passed" << std::endl;
        } else {
            std::cout << "❌ Conversation insertion test failed" << std::endl;
        }
        
        // 查询会话
        auto conversations = db_manager.get_all_conversations();
        if (conversations.hasValue() && conversations->size() >= 1) {
            std::cout << "✅ Conversation query test passed, found " << conversations->size() << " conversations" << std::endl;
        } else {
            std::cout << "❌ Conversation query test failed" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ WCDB basic operations test failed: " << e.what() << std::endl;
    }
}

// 测试 WCDB 头文件是否可用
inline void test_wcdb_headers() {
    std::cout << "Testing WCDB headers availability..." << std::endl;
    
    // 检查 WCDB 头文件
    #ifdef WCDB_VERSION
        std::cout << "WCDB version: " << WCDB_VERSION << std::endl;
    #else
        std::cout << "WCDB headers found (version not available)" << std::endl;
    #endif
    
    std::cout << "✅ WCDB headers test completed" << std::endl;
}

// 运行所有 WCDB 测试
inline void run_wcdb_tests() {
    std::cout << "=== WCDB Integration Tests ===" << std::endl;
    
    test_wcdb_headers();
    test_wcdb_linking();
    test_wcdb_basic_operations();
    
    std::cout << "=== WCDB Tests Completed ===" << std::endl;
}

#endif // ROC_TEST_WCDB_TEST_H 