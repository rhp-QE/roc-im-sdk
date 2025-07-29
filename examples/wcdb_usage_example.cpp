///
/// @file   wcdb_usage_example.cpp
/// @brief  WCDB 使用示例
/// @author RuanHuipeng
/// @date   2025-07-23
/// @version 1.0
///

#include "base/database/WCDBExample.h"
#include <iostream>

/**
 * @brief WCDB 使用示例
 * 
 * 这个示例展示了如何使用 WCDB 进行基本的数据库操作
 * 参考官方教程：https://github.com/Tencent/wcdb/wiki/C++-%e5%bf%ab%e9%80%9f%e5%85%a5%e9%97%a8
 */
void wcdb_usage_example() {
    std::cout << "=== WCDB Usage Example ===" << std::endl;
    
    try {
        // 创建数据库管理器
        roc::base::database::WCDBManager db_manager;
        
        // 初始化数据库
        std::string db_path = "example.db";
        if (!db_manager.init_database(db_path)) {
            std::cout << "❌ Failed to initialize database" << std::endl;
            return;
        }
        std::cout << "✅ Database initialized: " << db_path << std::endl;
        
        // 创建表
        if (!db_manager.create_tables()) {
            std::cout << "❌ Failed to create tables" << std::endl;
            return;
        }
        std::cout << "✅ Tables created successfully" << std::endl;
        
        // 插入消息数据
        std::cout << "\n--- Inserting Messages ---" << std::endl;
        roc::base::database::Message msg1(1, "Hello WCDB!");
        roc::base::database::Message msg2(2, "This is a test message");
        roc::base::database::Message msg3(3, "WCDB is awesome!");
        
        if (db_manager.insert_message(msg1)) {
            std::cout << "✅ Inserted message: " << msg1.content << std::endl;
        }
        if (db_manager.insert_message(msg2)) {
            std::cout << "✅ Inserted message: " << msg2.content << std::endl;
        }
        if (db_manager.insert_message(msg3)) {
            std::cout << "✅ Inserted message: " << msg3.content << std::endl;
        }
        
        // 查询所有消息
        std::cout << "\n--- Querying Messages ---" << std::endl;
        auto messages = db_manager.get_all_messages();
        if (messages.hasValue()) {
            std::cout << "Found " << messages->size() << " messages:" << std::endl;
            for (const auto& msg : *messages) {
                std::cout << "  ID: " << msg.identifier << ", Content: " << msg.content << std::endl;
            }
        } else {
            std::cout << "No messages found" << std::endl;
        }
        
        // 插入会话数据
        std::cout << "\n--- Inserting Conversations ---" << std::endl;
        roc::base::database::Conversation conv1("conv_001", "Test Conversation 1", 1234567890);
        roc::base::database::Conversation conv2("conv_002", "Test Conversation 2", 1234567891);
        
        if (db_manager.insert_conversation(conv1)) {
            std::cout << "✅ Inserted conversation: " << conv1.name << std::endl;
        }
        if (db_manager.insert_conversation(conv2)) {
            std::cout << "✅ Inserted conversation: " << conv2.name << std::endl;
        }
        
        // 查询所有会话
        std::cout << "\n--- Querying Conversations ---" << std::endl;
        auto conversations = db_manager.get_all_conversations();
        if (conversations.hasValue()) {
            std::cout << "Found " << conversations->size() << " conversations:" << std::endl;
            for (const auto& conv : *conversations) {
                std::cout << "  ID: " << conv.conv_id << ", Name: " << conv.name 
                          << ", LastMsgTime: " << conv.last_msg_time << std::endl;
            }
        } else {
            std::cout << "No conversations found" << std::endl;
        }
        
        std::cout << "\n✅ WCDB usage example completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ WCDB usage example failed: " << e.what() << std::endl;
    }
} 