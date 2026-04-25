#include "SpdlogAdapter.h"
#include <filesystem>
#include <iostream>

namespace roc::imsdk {
namespace {

std::string normalize_log_dir(const std::string& log_dir) {
    const std::filesystem::path logs_root("logs");
    if (log_dir.empty()) {
        return logs_root.string();
    }

    const std::filesystem::path input_path(log_dir);

    // 统一将日志目录收敛到项目根目录的 logs/ 下
    if (input_path.is_absolute()) {
        return (logs_root / input_path.filename()).string();
    }

    auto begin = input_path.begin();
    if (begin != input_path.end() && *begin == logs_root) {
        return input_path.string();
    }

    return (logs_root / input_path).string();
}

} // namespace

SpdlogAdapter::SpdlogAdapter() : logDir_("logs"), stop_flush_(false), flush_running_(false) {
    initialize_default_logger();
}

SpdlogAdapter::SpdlogAdapter(const std::string& logDir)
    : logDir_(normalize_log_dir(logDir)), stop_flush_(false), flush_running_(false) {
    initialize_default_logger();
}

SpdlogAdapter::SpdlogAdapter(std::shared_ptr<spdlog::logger> logger) 
    : spdlogLogger_(std::move(logger)), logDir_("logs"), stop_flush_(false), flush_running_(false) {
    if (!spdlogLogger_) {
        initialize_default_logger();
    }
}

SpdlogAdapter::~SpdlogAdapter() {
    // 确保定期刷新协程安全停止
    stop_periodic_flush();
}

void SpdlogAdapter::initialize_default_logger() {
    try {
        // 创建日志目录
        create_log_directory();
        
        // 创建异步日志器
        auto async_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
            "async_logger", 
            logDir_ + "/roc_im_sdk_async.log", 
            1024*1024*10, // 10MB
            5              // 保留5个文件
        );
        async_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        async_logger->set_level(spdlog::level::debug);
        
        // 使用异步日志器作为默认日志器
        spdlogLogger_ = async_logger;
        
        // 设置全局默认日志器
        spdlog::set_default_logger(async_logger);
        
        spdlogLogger_->info("=== SpdlogAdapter 初始化完成 ===");
        
    } catch (const std::exception& e) {
        std::cerr << "SpdlogAdapter 初始化失败: " << e.what() << std::endl;
        // 如果初始化失败，创建一个简单的控制台日志器
        spdlogLogger_ = spdlog::stdout_color_mt("fallback");
        spdlogLogger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
    }
}

void SpdlogAdapter::create_log_directory() {
    try {
        if (!std::filesystem::exists(logDir_)) {
            std::filesystem::create_directories(logDir_);
        }
    } catch (const std::exception& e) {
        std::cerr << "创建日志目录失败: " << e.what() << std::endl;
    }
}

std::shared_ptr<ILogger> SpdlogAdapter::get_logger() const {
    auto logger = std::make_shared<ILogger>();
    
    logger->set_log_function([this](LogLevel level, const std::string& module, const std::string& message) {
        if (!spdlogLogger_) return;
        
        // 构建完整的日志消息，包含模块名
        std::string fullMessage = "【" + module + "】" + message;
        
        // 根据级别调用相应的 spdlog 方法
        switch (level) {
            case LogLevel::Trace:
                spdlogLogger_->trace("" + fullMessage);
                break;
            case LogLevel::Debug:
                spdlogLogger_->debug("" + fullMessage);
                break;
            case LogLevel::Info:
                spdlogLogger_->info(" " + fullMessage);
                break;
            case LogLevel::Warn:
                spdlogLogger_->warn(" " + fullMessage);
                break;
            case LogLevel::Error:
                spdlogLogger_->error(fullMessage);
                break;
            case LogLevel::Critical:
                spdlogLogger_->critical(fullMessage);
                break;
            default:
                spdlogLogger_->info(" " + fullMessage);
                break;
        }
    });
    
    // 注入级别设置函数
    logger->set_level_function([this](LogLevel level) {
        if (spdlogLogger_) {
            spdlogLogger_->set_level(convert_level(level));
        }
    });
    
    // 注入级别获取函数
    logger->set_get_level_function([this]() -> LogLevel {
        if (spdlogLogger_) {
            return convert_level(spdlogLogger_->level());
        }
        return LogLevel::Info;
    });
    
    // 注入级别检查函数
    logger->set_should_log_function([this](LogLevel level) -> bool {
        if (spdlogLogger_) {
            return spdlogLogger_->should_log(convert_level(level));
        }
        return false;
    });
    
    // 注入刷新函数
    logger->set_flush_function([this]() {
        if (spdlogLogger_) {
            spdlogLogger_->flush();
        }
    });
    
    return logger;
}

spdlog::level::level_enum SpdlogAdapter::convert_level(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return spdlog::level::trace;
        case LogLevel::Debug: return spdlog::level::debug;
        case LogLevel::Info: return spdlog::level::info;
        case LogLevel::Warn: return spdlog::level::warn;
        case LogLevel::Error: return spdlog::level::err;
        case LogLevel::Critical: return spdlog::level::critical;
        case LogLevel::Off: return spdlog::level::off;
        default: return spdlog::level::info;
    }
}

LogLevel SpdlogAdapter::convert_level(spdlog::level::level_enum level) {
    switch (level) {
        case spdlog::level::trace: return LogLevel::Trace;
        case spdlog::level::debug: return LogLevel::Debug;
        case spdlog::level::info: return LogLevel::Info;
        case spdlog::level::warn: return LogLevel::Warn;
        case spdlog::level::err: return LogLevel::Error;
        case spdlog::level::critical: return LogLevel::Critical;
        case spdlog::level::off: return LogLevel::Off;
        default: return LogLevel::Info;
    }
}

// 静态工厂函数实现
std::shared_ptr<SpdlogAdapter> SpdlogAdapter::create(const std::string& logDir) {
    return std::make_shared<SpdlogAdapter>(logDir);
}

std::shared_ptr<SpdlogAdapter> SpdlogAdapter::create(std::shared_ptr<spdlog::logger> logger) {
    return std::make_shared<SpdlogAdapter>(std::move(logger));
}

// 启动定期日志刷新（使用协程）
void SpdlogAdapter::start_periodic_flush_async(std::shared_ptr<boost::asio::io_context> io_context, int interval_ms) {
    if (flush_running_.load()) {
        return; // 已经在运行
    }
    
    if (!io_context) {
        if (spdlogLogger_) {
            spdlogLogger_->error("io_context 为空，无法启动定期刷新");
        }
        return;
    }
    
    io_context_ = io_context;
    stop_flush_.store(false);
    flush_running_.store(true);
    
    if (spdlogLogger_) {
        spdlogLogger_->info("定期日志刷新已启动，间隔: {}ms", interval_ms);
    }
    
    // 在内部启动协程
    boost::asio::co_spawn(*io_context_, 
        [this, interval_ms]() -> boost::asio::awaitable<void> {
            co_await this->periodic_flush_coroutine(interval_ms);
        }, 
        boost::asio::detached
    );
}

// 停止定期日志刷新
void SpdlogAdapter::stop_periodic_flush() {
    if (!flush_running_.load()) {
        return; // 没有在运行
    }
    
    stop_flush_.store(true);
    
    if (spdlogLogger_) {
        spdlogLogger_->info("定期日志刷新停止信号已发送");
    }
}

// 检查定期刷新是否正在运行
bool SpdlogAdapter::is_periodic_flush_running() const {
    return flush_running_.load();
}

// 定期刷新协程函数
boost::asio::awaitable<void> SpdlogAdapter::periodic_flush_coroutine(int interval_ms) {
    boost::asio::steady_timer timer(*io_context_);

    auto self = shared_from_this();

    while (!stop_flush_.load()) {
        // 执行日志刷新
        if (spdlogLogger_) {
            spdlogLogger_->flush();
        }
        
        // 等待指定间隔
        timer.expires_after(std::chrono::milliseconds(interval_ms));
        co_await timer.async_wait(boost::asio::use_awaitable);
    }
    
    // 协程结束，更新状态
    flush_running_.store(false);
    
    if (spdlogLogger_) {
        spdlogLogger_->info("定期日志刷新协程已结束");
    }
}

} // namespace roc::imsdk
