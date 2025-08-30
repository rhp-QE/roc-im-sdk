#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

/**
 * spdlog 使用示例
 * 展示如何在 ROC IM SDK 项目中使用 spdlog 进行日志记录
 */

void basic_logging_example() {
    std::cout << "\n=== 基本日志记录示例 ===" << std::endl;
    
    // 使用默认日志器
    spdlog::info("欢迎使用 ROC IM SDK!");
    spdlog::warn("这是一个警告消息");
    spdlog::error("这是一个错误消息");
    spdlog::critical("这是一个严重错误消息");
    
    // 格式化日志
    spdlog::info("当前时间: {}", spdlog::fmt_lib::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::system_clock::now()));
    spdlog::info("用户ID: {}, 状态: {}", 12345, "在线");
}

void custom_logger_example() {
    std::cout << "\n=== 自定义日志器示例 ===" << std::endl;
    
    // 创建 logs 目录
    std::filesystem::create_directories("logs");
    
    // 创建控制台彩色日志器
    auto console_logger = spdlog::stdout_color_mt("console");
    console_logger->set_level(spdlog::level::debug);
    console_logger->info("这是控制台日志器的消息");
    console_logger->debug("这是调试信息");
    
    // 创建文件日志器
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/roc_im_sdk.log");
    auto file_logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
    spdlog::register_logger(file_logger);
    file_logger->set_level(spdlog::level::trace);
    
    // 写入多条日志确保文件有内容
    file_logger->info("=== 文件日志器测试开始 ===");
    file_logger->info("这是文件日志器的消息");
    file_logger->trace("这是跟踪信息");
    file_logger->debug("这是调试信息");
    file_logger->warn("这是警告信息");
    file_logger->error("这是错误信息");
    file_logger->info("=== 文件日志器测试结束 ===");
    
    // 强制刷新到文件
    file_logger->flush();
    
    // 创建轮转文件日志器
    auto rotating_logger = spdlog::rotating_logger_mt("rotating_logger", 
                                                     "logs/roc_im_sdk_rotating.log", 
                                                     1024*1024*5, // 5MB
                                                     3);          // 保留3个文件
    rotating_logger->set_level(spdlog::level::debug);
    rotating_logger->info("=== 轮转日志器测试开始 ===");
    rotating_logger->info("这是轮转文件日志器的消息");
    rotating_logger->debug("轮转日志器调试信息");
    rotating_logger->warn("轮转日志器警告信息");
    rotating_logger->info("=== 轮转日志器测试结束 ===");
    rotating_logger->flush();
    
    // 创建每日文件日志器
    auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/roc_im_sdk_daily.log");
    daily_logger->set_level(spdlog::level::debug);
    daily_logger->info("=== 每日日志器测试开始 ===");
    daily_logger->info("这是每日文件日志器的消息");
    daily_logger->debug("每日日志器调试信息");
    daily_logger->warn("每日日志器警告信息");
    daily_logger->info("=== 每日日志器测试结束 ===");
    daily_logger->flush();
    

}

void async_logging_example() {
    std::cout << "\n=== 异步日志记录示例 ===" << std::endl;
    
    // 创建异步日志器
    auto async_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
        "async_logger", 
        "logs/roc_im_sdk_async.log", 
        1024*1024*10, // 10MB
        5              // 保留5个文件
    );
    
    async_logger->set_level(spdlog::level::debug);
    
    // 异步记录大量日志
    async_logger->info("=== 异步日志器测试开始 ===");
    for (int i = 0; i < 100; ++i) {  // 减少到100条避免文件过大
        async_logger->info("异步日志消息 #{}", i);
        async_logger->debug("name = {}, id = {}, flat = {}, bool = {}", "uuu", 100, 100.1, true);
    }

    async_logger->info("=== 异步日志器测试结束 ===");
    
    // 强制刷新异步日志器
    async_logger->flush();
    
    // 等待异步日志写入完成（增加等待时间）
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    spdlog::info("异步日志记录完成");
}

void custom_format_example() {
    std::cout << "\n=== 自定义格式示例 ===" << std::endl;
    
    // 创建自定义格式的日志器
    auto custom_logger = spdlog::stdout_color_mt("custom_format");
    
    // 设置自定义格式：时间 | 级别 | 模块 | 消息
    custom_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
    
    custom_logger->info("用户登录成功");
    custom_logger->warn("网络连接不稳定");
    custom_logger->error("数据库连接失败");
}

void logger_management_example() {
    std::cout << "\n=== 日志器管理示例 ===" << std::endl;
    
    // 获取已存在的日志器
    auto console = spdlog::get("console");
    if (console) {
        console->info("获取到已存在的控制台日志器");
    }
    
    // 设置全局日志级别
    spdlog::set_level(spdlog::level::debug);
    
    // 设置全局错误处理器
    spdlog::set_error_handler([](const std::string& msg) {
        std::cerr << "日志错误: " << msg << std::endl;
    });
    
    // 刷新所有日志器
    // spdlog::flush_all();
    
    spdlog::info("日志器管理示例完成");
}

int test_logger() {
    std::cout << "🚀 ROC IM SDK - spdlog 日志系统示例" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    try {
        // 设置默认日志器
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
        
        basic_logging_example();
        custom_logger_example();
        async_logging_example();
        custom_format_example();
        logger_management_example();
        
        std::cout << "\n✅ 所有示例执行完成！" << std::endl;
        std::cout << "📁 日志文件已保存到 logs/ 目录" << std::endl;
        
        // 强制刷新所有日志器
        std::cout << "🔄 正在刷新所有日志器..." << std::endl;
        
        // 获取并刷新所有已注册的日志器
        auto async_logger = spdlog::get("async_logger");
        if (async_logger) {
            async_logger->flush();
        }
        
        // 额外等待确保异步日志完成
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "✅ 日志刷新完成" << std::endl;
        
        // 检查日志文件状态
        std::cout << "\n📊 日志文件状态检查:" << std::endl;
        std::vector<std::string> logFiles = {
            "logs/roc_im_sdk.log",
            "logs/roc_im_sdk_rotating.log", 
            "logs/roc_im_sdk_daily.log",
            "logs/roc_im_sdk_async.log"
        };
        
        for (const auto& file : logFiles) {
            if (std::filesystem::exists(file)) {
                auto size = std::filesystem::file_size(file);
                std::cout << "  ✅ " << file << " - " << size << " bytes" << std::endl;
            } else {
                std::cout << "  ❌ " << file << " - 文件不存在" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
