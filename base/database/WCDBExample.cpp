#include "base/database/WCDBExample.h"
#include <iostream>

namespace roc::base::database {

// Message 模型实现
Message::Message() : identifier(0), content("") {
}

Message::Message(int identifier, const std::string& content) 
    : identifier(identifier), content(content) {
}

// Conversation 模型实现
Conversation::Conversation() : conv_id(""), name(""), last_msg_time(0) {
}

Conversation::Conversation(const std::string& conv_id, const std::string& name, int64_t last_msg_time)
    : conv_id(conv_id), name(name), last_msg_time(last_msg_time) {
}

// WCDB ORM 实现 - 按照官方教程的固定模板
// 将 Message 类的 identifier 和 content 两个变量绑定到表中同名字段
WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(Message)
WCDB_CPP_SYNTHESIZE(identifier)
WCDB_CPP_SYNTHESIZE(content)
WCDB_CPP_ORM_IMPLEMENTATION_END

// 将 Conversation 类的 conv_id、name 和 last_msg_time 三个变量绑定到表中同名字段
WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(Conversation)
WCDB_CPP_SYNTHESIZE(conv_id)
WCDB_CPP_SYNTHESIZE(name)
WCDB_CPP_SYNTHESIZE(last_msg_time)
WCDB_CPP_ORM_IMPLEMENTATION_END

// WCDBManager 实现
WCDBManager::WCDBManager() : is_initialized_(false) {
}

WCDBManager::~WCDBManager() = default;

bool WCDBManager::init_database(const std::string& db_path) {
    if (is_initialized_) {
        std::cerr << "Database already initialized" << std::endl;
        return false;
    }

    try {
        // 按照官方教程：One line of code 创建数据库
        database_ = std::make_unique<WCDB::Database>(db_path);
        is_initialized_ = true;
        std::cout << "Database initialized successfully: " << db_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize database: " << e.what() << std::endl;
        return false;
    }
}

bool WCDBManager::create_tables() {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    try {
        // 按照官方教程：One line of code 创建表
        // 以下代码等效于 SQL：CREATE TABLE IF NOT EXISTS messageTable(identifier INTEGER, content TEXT)
        bool ret = database_->createTable<Message>("messageTable");
        
        // 创建会话表
        ret &= database_->createTable<Conversation>("conversationTable");
        
        if (ret) {
            std::cout << "Tables created successfully" << std::endl;
        } else {
            std::cerr << "Failed to create tables" << std::endl;
        }
        
        return ret;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create tables: " << e.what() << std::endl;
        return false;
    }
}

bool WCDBManager::insert_message(const Message& message) {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    try {
        // 按照官方教程：One line of code 插入数据
        // 以下代码等效于 SQL：INSERT INTO messageTable(identifier, content) VALUES(message.identifier, message.content)
        bool ret = database_->insertObjects<Message>(message, "messageTable");
        
        if (ret) {
            std::cout << "Message inserted successfully: " << message.identifier << std::endl;
        } else {
            std::cerr << "Failed to insert message" << std::endl;
        }
        
        return ret;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert message: " << e.what() << std::endl;
        return false;
    }
}

WCDB::OptionalValueArray<Message> WCDBManager::get_all_messages() {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return WCDB::OptionalValueArray<Message>();
    }

    try {
        // 按照官方教程：One line of code 查询数据
        // 以下代码等效于 SQL：SELECT identifier, content from messageTable
        WCDB::OptionalValueArray<Message> objects = database_->getAllObjects<Message>("messageTable");
        
        if (objects.hasValue()) {
            std::cout << "Retrieved " << objects->size() << " messages" << std::endl;
        } else {
            std::cout << "No messages found or query failed" << std::endl;
        }
        return objects;
    } catch (const std::exception& e) {
        std::cerr << "Failed to get messages: " << e.what() << std::endl;
        return WCDB::OptionalValueArray<Message>();
    }
}

bool WCDBManager::update_message(const Message& message, const WCDB::Expression& condition) {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    try {
        // 按照官方教程：One line of code 更新数据
        // 以下代码等效于 SQL：UPDATE messageTable SET content = message.content WHERE condition
        bool ret = database_->updateObject(message,
                                          WCDB_FIELD(Message::content),
                                          "messageTable",
                                          condition);
        
        if (ret) {
            std::cout << "Message updated successfully" << std::endl;
        } else {
            std::cerr << "Failed to update message" << std::endl;
        }
        
        return ret;
    } catch (const std::exception& e) {
        std::cerr << "Failed to update message: " << e.what() << std::endl;
        return false;
    }
}

bool WCDBManager::delete_messages(const WCDB::Expression& condition) {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    try {
        // 按照官方教程：One line of code 删除数据
        // 以下代码等效于 SQL：DELETE FROM messageTable WHERE condition
        bool ret = database_->deleteObjects("messageTable", condition);
        
        if (ret) {
            std::cout << "Messages deleted successfully" << std::endl;
        } else {
            std::cerr << "Failed to delete messages" << std::endl;
        }
        
        return ret;
    } catch (const std::exception& e) {
        std::cerr << "Failed to delete messages: " << e.what() << std::endl;
        return false;
    }
}

bool WCDBManager::insert_conversation(const Conversation& conversation) {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    try {
        bool ret = database_->insertObjects<Conversation>(conversation, "conversationTable");
        
        if (ret) {
            std::cout << "Conversation inserted successfully: " << conversation.conv_id << std::endl;
        } else {
            std::cerr << "Failed to insert conversation" << std::endl;
        }
        
        return ret;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert conversation: " << e.what() << std::endl;
        return false;
    }
}

WCDB::OptionalValueArray<Conversation> WCDBManager::get_all_conversations() {
    if (!is_initialized_ || !database_) {
        std::cerr << "Database not initialized" << std::endl;
        return WCDB::OptionalValueArray<Conversation>();
    }

    try {
        WCDB::OptionalValueArray<Conversation> objects = database_->getAllObjects<Conversation>("conversationTable");
        
        if (objects.hasValue()) {
            std::cout << "Retrieved " << objects->size() << " conversations" << std::endl;
        } else {
            std::cout << "No conversations found or query failed" << std::endl;
        }
        return objects;
    } catch (const std::exception& e) {
        std::cerr << "Failed to get conversations: " << e.what() << std::endl;
        return WCDB::OptionalValueArray<Conversation>();
    }
}

} // namespace roc::base::database 