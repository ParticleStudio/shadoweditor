#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <format>
#include <source_location>

#include "common/string.hpp"
#include "logger/global_logger.h"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace shadow::logger {
    GlobalLogger::~GlobalLogger() noexcept {
    }

    void GlobalLogger::Init(const std::string_view &rLogPath, const LogLevel logLevel, const int32_t qsize, const int32_t threadNum, const int32_t backtraceNum) {
        try {
            if (!m_isInitialized) {
                std::scoped_lock<std::mutex> const lock(m_mutex);

                spdlog::init_thread_pool(qsize, threadNum);
                CreateMainLogger(logLevel, util::StrCat(rLogPath, "/main.log"), backtraceNum);
                CreateErrorLogger(util::StrCat(rLogPath, "/error.log"), backtraceNum);

                m_isInitialized = true;
            }
        } catch (const spdlog::spdlog_ex &ex) {
            std::cout << "log init failed:" << ex.what() << std::endl;

            Stop();
        }
    }

    void GlobalLogger::CreateMainLogger(const LogLevel logLevel, const std::string_view &rLogFile, const int32_t backtraceNum) {
        auto pSinkStdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto pSinkHourly = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(rLogFile.data(), 0, 0);
        spdlog::sinks_init_list sinks{pSinkStdout, pSinkHourly};
        this->m_pMainLogger = std::make_shared<spdlog::async_logger>("mainLogger", sinks, spdlog::thread_pool());
        this->m_pMainLogger->set_pattern(m_pattern.data());
        this->m_pMainLogger->enable_backtrace(backtraceNum);
        SetLogLevel(logLevel);

        spdlog::register_logger(this->m_pMainLogger);
    }

    void GlobalLogger::CreateErrorLogger(const std::string_view &rLogFile, int32_t backtraceNum) {
        this->m_pErrorLogger = spdlog::hourly_logger_mt("errorLog", rLogFile.data(), false, 0);
        this->m_pErrorLogger->set_pattern(m_pattern.data());
        this->m_pErrorLogger->enable_backtrace(backtraceNum);
        this->m_pErrorLogger->set_level(spdlog::level::level_enum::err);
    }

    void GlobalLogger::SetLogLevel(const LogLevel logLevel) const {
        switch (logLevel) {
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

    void GlobalLogger::Stop() {
        try {
            std::scoped_lock<std::mutex> const lock(m_mutex);

            if (!m_isInitialized) {
                std::cout << "logger stop failed: not initialized\n";
                return;
            }

            if (this->m_pMainLogger != nullptr or this->m_pErrorLogger != nullptr) {
                if (this->m_pMainLogger != nullptr) {
                    this->m_pMainLogger->flush();
                    this->m_pMainLogger.reset();
                }
                if (this->m_pErrorLogger != nullptr) {
                    this->m_pErrorLogger->flush();
                    this->m_pErrorLogger.reset();
                }

                spdlog::shutdown();
            }

            m_isInitialized = false;
        } catch (const spdlog::spdlog_ex &ex) {
            std::cout << "logger stop failed: " << ex.what() << '\n';
        }
    }

    void GlobalLogger::Trace(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->trace(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }

    void GlobalLogger::Debug(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->debug(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }

    void GlobalLogger::Info(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->info(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }

    void GlobalLogger::Warning(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->warn(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }

    void GlobalLogger::Error(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
        this->m_pErrorLogger->error(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }

    void GlobalLogger::Critical(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pMainLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
        this->m_pErrorLogger->critical(std::format("[{}:{}][{}] {}", rLocation.file_name(), rLocation.line(), rLocation.function_name(), msg.data()));
    }
} // namespace shadow::logger
