#include "logger/logger_manager.h"

#include <ctime>
#include <iostream>

#include "common/string.hpp"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace logger {
void LoggerManager::Init(const std::string_view &rLogPath, LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    try {
        spdlog::init_thread_pool(qsize, threadNum);
        CreateMainLogger(logLevel, util::StrCat(rLogPath, "/main.log"), backtraceNum);
        CreateErrorLogger(util::StrCat(rLogPath, "/error.log"), backtraceNum);
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log init failed:" << ex.what() << std::endl;

        Release();
    }
}

void LoggerManager::CreateMainLogger(LogLevel logLevel, const std::string_view &rLogFile, int32_t backtraceNum) {
    auto pSinkStdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto pSinkHourly = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(rLogFile.data(), 0, 0);
    spdlog::sinks_init_list sinks{pSinkStdout, pSinkHourly};
    this->m_pMainLogger = std::make_shared<spdlog::async_logger>("mainLogger", sinks, spdlog::thread_pool());
    this->m_pMainLogger->set_pattern(m_pattern.data());
    this->m_pMainLogger->enable_backtrace(backtraceNum);
    SetLogLevel(logLevel);

    spdlog::register_logger(this->m_pMainLogger);
}

void LoggerManager::CreateErrorLogger(const std::string_view &rLogFile, int32_t backtraceNum) {
    this->m_pErrorLogger = spdlog::hourly_logger_mt("errorLog", rLogFile.data(), false, 0);
    this->m_pErrorLogger->set_pattern(m_pattern.data());
    this->m_pErrorLogger->enable_backtrace(backtraceNum);
    this->m_pErrorLogger->set_level(spdlog::level::level_enum::err);
}

void LoggerManager::SetLogLevel(LogLevel logLevel) {
    switch(logLevel) {
        case LogLevel::Trace:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::trace);
        case LogLevel::Debug:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::debug);
        case LogLevel::Info:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::info);
        case LogLevel::Warn:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::warn);
        case LogLevel::Error:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::err);
        case LogLevel::Critical:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::critical);
        case LogLevel::Off:
            return this->m_pMainLogger->set_level(spdlog::level::level_enum::off);
        default:
            return;
    }
}

void LoggerManager::Release() {
    try {
        if(this->m_pMainLogger.get() != nullptr or this->m_pErrorLogger.get() != nullptr) {
            spdlog::shutdown();

            this->m_pMainLogger.reset();
            this->m_pErrorLogger.reset();
        }
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log release failed:" << ex.what() << std::endl;
    }
}

void LoggerManager::LogTrace(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->trace(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LoggerManager::LogDebug(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->debug(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LoggerManager::LogInfo(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->info(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LoggerManager::LogWarning(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->warn(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LoggerManager::LogError(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    this->m_pErrorLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}

void LoggerManager::LogCritical(const std::string_view &msg, std::source_location &&rLocation) {
    this->m_pMainLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    this->m_pErrorLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
}
} // namespace logger
