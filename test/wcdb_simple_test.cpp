///
/// @file   wcdb_simple_test.cpp
/// @brief  简化的 WCDB 测试程序，严格按照官方教程实现
/// @author RuanHuipeng
/// @date   2025-07-23
/// @version 1.0
///

#include <WCDB/WCDBCpp.h>
#include <iostream>
#include <string>

// 严格按照官方教程的 Sample 类实现
class Sample {
public:
    Sample(); // 必须要有默认构造函数
    Sample(int identifier, const std::string& content); // 非必须实现的构造函数
    
    int identifier_r;
    std::string content_r;
    
    WCDB_CPP_ORM_DECLARATION(Sample)
};

// 构造函数实现
Sample::Sample() : identifier_r(0), content_r("") {
}

Sample::Sample(int identifier, const std::string& content) 
    : identifier_r(identifier), content_r(content) {
}

// ORM 绑定实现 - 严格按照官方教程
WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(Sample)
WCDB_CPP_SYNTHESIZE(identifier_r)
WCDB_CPP_SYNTHESIZE(content_r)
WCDB_CPP_ORM_IMPLEMENTATION_END

/**
 * @brief 简化的 WCDB 测试，严格按照官方教程实现
 */
void wcdb_simple_test() {
    std::cout << "=== WCDB Simple Test (Following Official Tutorial) ===" << std::endl;
    
    try {
        // 1. 创建数据库对象 - 严格按照官方教程
        std::cout << "1. Creating database..." << std::endl;
        WCDB::Database database("/root/project/ROCIM/dbData/simple_test.db");
        std::cout << "✅ Database created successfully" << std::endl;
        
        // 2. 创建表 - 严格按照官方教程
        std::cout << "2. Creating table..." << std::endl;
        // 以下代码等效于 SQL：CREATE TABLE IF NOT EXISTS sampleTable(identifier INTEGER, content TEXT)
        bool ret = database.createTable<Sample>("sampleTable");
        if (ret) {
            std::cout << "✅ Table created successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to create table" << std::endl;
            return;
        }
        
        // 3. 插入数据 - 严格按照官方教程
        std::cout << "3. Inserting data..." << std::endl;
        // 以下代码等效于 SQL：INSERT INTO sampleTable(identifier, content) VALUES(1, sample_insert)
        ret = database.insertObjects<Sample>(Sample(1, "sample_insert"), "sampleTable");
        if (ret) {
            std::cout << "✅ Data inserted successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to insert data" << std::endl;
            return;
        }
        
        // 4. 查询数据 - 严格按照官方教程
        std::cout << "4. Querying data..." << std::endl;
        // 以下代码等效于 SQL：SELECT identifier, content from sampleTable
        WCDB::OptionalValueArray<Sample> objects = database.getAllObjects<Sample>("sampleTable");
        if (objects.hasValue()) {
            std::cout << "✅ Query successful, found " << objects->size() << " objects" << std::endl;
            for (const auto& obj : *objects) {
                std::cout << "  ID: " << obj.identifier_r << ", Content: " << obj.content_r << std::endl;
            }
        } else {
            std::cout << "❌ Query failed or no data found" << std::endl;
        }
        
        // 5. 更新数据 - 严格按照官方教程
        std::cout << "5. Updating data..." << std::endl;
        Sample updateObject(0, "sample_update");
        // 以下代码等效于 SQL：UPDATE sampleTable SET content = sample_update WHERE identifier > 0
        ret = database.updateObject(updateObject,
                                   WCDB_FIELD(Sample::content_r),
                                   "sampleTable",
                                   WCDB_FIELD(Sample::identifier_r) > 0);
        if (ret) {
            std::cout << "✅ Data updated successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to update data" << std::endl;
        }
        
        // 6. 删除数据 - 严格按照官方教程
        std::cout << "6. Deleting data..." << std::endl;
        // 以下代码等效于 SQL：DELETE FROM sampleTable
        ret = database.deleteObjects("sampleTable");
        if (ret) {
            std::cout << "✅ Data deleted successfully" << std::endl;
        } else {
            std::cout << "❌ Failed to delete data" << std::endl;
        }
        
        std::cout << "\n✅ WCDB simple test completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ WCDB simple test failed: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "❌ WCDB simple test failed with unknown error" << std::endl;
    }
} 