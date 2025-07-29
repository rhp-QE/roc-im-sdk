#ifndef ROC_BASE_DATABASE_WCDB_EXAMPLE_H
#define ROC_BASE_DATABASE_WCDB_EXAMPLE_H

#include <WCDB/WCDBCpp.h>
#include <memory>
#include <string>
#include <vector>

namespace roc::base::database {

/**
 * @brief 消息模型 - 按照 WCDB 官方教程的 Sample 类模式
 */
class Message {
public:
    Message(); // 必须要有默认构造函数
    Message(int identifier, const std::string& content); // 非必须实现的构造函数
    
    int identifier;
    std::string content;
    
    WCDB_CPP_ORM_DECLARATION(Message)
};

/**
 * @brief 会话模型
 */
class Conversation {
public:
    Conversation();
    Conversation(const std::string& conv_id, const std::string& name, int64_t last_msg_time);
    
    std::string conv_id;
    std::string name;
    int64_t last_msg_time;
    
    WCDB_CPP_ORM_DECLARATION(Conversation)
};

/**
 * @brief WCDB 数据库管理器示例
 * 
 * 这个类展示了如何在项目中使用 WCDB 数据库
 * 参考官方教程：https://github.com/Tencent/wcdb/wiki/C++-%e5%bf%ab%e9%80%9f%e5%85%a5%e9%97%a8
 */
class WCDBManager {
public:
    WCDBManager();
    ~WCDBManager();

    /**
     * @brief 初始化数据库
     * @param db_path 数据库文件路径
     * @return 是否成功
     */
    bool init_database(const std::string& db_path);

    /**
     * @brief 创建表
     * @return 是否成功
     */
    bool create_tables();

    /**
     * @brief 插入消息
     * @param message 消息对象
     * @return 是否成功
     */
    bool insert_message(const Message& message);

    /**
     * @brief 获取所有消息
     * @return 消息列表
     */
    WCDB::OptionalValueArray<Message> get_all_messages();

    /**
     * @brief 更新消息
     * @param message 消息对象
     * @param condition 更新条件
     * @return 是否成功
     */
    bool update_message(const Message& message, const WCDB::Expression& condition);

    /**
     * @brief 删除消息
     * @param condition 删除条件
     * @return 是否成功
     */
    bool delete_messages(const WCDB::Expression& condition);

    /**
     * @brief 插入会话
     * @param conversation 会话对象
     * @return 是否成功
     */
    bool insert_conversation(const Conversation& conversation);

    /**
     * @brief 获取所有会话
     * @return 会话列表
     */
    WCDB::OptionalValueArray<Conversation> get_all_conversations();

private:
    std::unique_ptr<WCDB::Database> database_;
    bool is_initialized_;
};

} // namespace roc::base::database

#endif // ROC_BASE_DATABASE_WCDB_EXAMPLE_H 