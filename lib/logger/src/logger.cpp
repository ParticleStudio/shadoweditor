#include "logger/logger.h"

#include "common/string.hpp"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace logger {
static std::shared_ptr<spdlog::logger> pMainLogger{nullptr};
static std::shared_ptr<spdlog::logger> pErrorLogger{nullptr};
static constexpr const std::string_view pattern = "%^[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v%$"; // "%^[%Y-%m-%d %H:%M:%S.%e][%t][%l][%s %!:%#] %v%$";

inline void CreateLogger(LogLevel logLevel, const std::string_view &rLogFile, int32_t backtraceNum) {
    auto pSinkStdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto pSinkHourly = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(rLogFile.data(), 0, 0);
    spdlog::sinks_init_list sinks{pSinkStdout, pSinkHourly};
    pMainLogger = std::make_shared<spdlog::async_logger>("mainLogger", sinks, spdlog::thread_pool());
    pMainLogger->set_level(static_cast<spdlog::level::level_enum>(logLevel));
    pMainLogger->set_pattern(pattern.data());
    pMainLogger->enable_backtrace(backtraceNum);

    spdlog::register_logger(pMainLogger);
}

inline void CreateLoggerError(const std::string_view &rLogFile, int32_t backtraceNum) {
    pErrorLogger = spdlog::hourly_logger_mt("errorLog", rLogFile.data(), false, 0);
    pErrorLogger->set_level(spdlog::level::level_enum::err);
    pErrorLogger->set_pattern(pattern.data());
    pErrorLogger->enable_backtrace(backtraceNum);
}

void Init(const std::string_view &rLogPath, LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    try {
        spdlog::init_thread_pool(qsize, threadNum);
        CreateLogger(logLevel, util::StrCat(rLogPath, "/main.log"), backtraceNum);
        CreateLoggerError(util::StrCat(rLogPath, "/error.log"), backtraceNum);
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log init failed:" << ex.what() << std::endl;

        Release();
    }
}

void Release() {
    try {
        spdlog::drop_all();
        spdlog::shutdown();
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log release failed:" << ex.what() << std::endl;
    }
}

[[maybe_unused]] void SetLogLevel(LogLevel logLevel) {
    switch(logLevel) {
        case LogLevel::Trace:
            return pMainLogger->set_level(spdlog::level::level_enum::trace);
        case LogLevel::Debug:
            return pMainLogger->set_level(spdlog::level::level_enum::debug);
        case LogLevel::Info:
            return pMainLogger->set_level(spdlog::level::level_enum::info);
        case LogLevel::Warn:
            return pMainLogger->set_level(spdlog::level::level_enum::warn);
        case LogLevel::Error:
            return pMainLogger->set_level(spdlog::level::level_enum::err);
        case LogLevel::Critical:
            return pMainLogger->set_level(spdlog::level::level_enum::critical);
        case LogLevel::Off:
            return pMainLogger->set_level(spdlog::level::level_enum::off);
        default:
            return;
    }
}

void LogTrace(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->trace(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LogDebug(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->debug(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LogInfo(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->info(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    SPDLOG_LOGGER_INFO(pMainLogger, msg);
}

void LogWarning(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->warn(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LogError(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    pErrorLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LogCritical(const std::string_view &msg, std::source_location &&rLocation) {
    pMainLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    pErrorLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}
}// namespace logger