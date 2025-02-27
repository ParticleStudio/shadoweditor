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
                CreateLogger(logLevel, util::StrCat(rLogPath, "/main.log"), backtraceNum);

                m_isInitialized = true;
            }
        } catch (const spdlog::spdlog_ex &ex) {
            std::cout << "log init failed:" << ex.what() << std::endl;

            Stop();
        }
    }

    void GlobalLogger::CreateLogger(const LogLevel logLevel, const std::string_view &rLogFile, const int32_t backtraceNum) {
        auto pStdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto pHourlySink = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(rLogFile.data(), 0, 0);
        spdlog::sinks_init_list sinks{pStdoutSink, pHourlySink};
        this->m_pLogger = std::make_shared<spdlog::async_logger>("mainLogger", sinks, spdlog::thread_pool());
        this->m_pLogger->set_pattern(m_pattern.data());
        this->m_pLogger->enable_backtrace(backtraceNum);
        SetLogLevel(logLevel);

        spdlog::register_logger(this->m_pLogger);
    }

    void GlobalLogger::SetLogLevel(const LogLevel logLevel) const {
        switch (logLevel) {
            case LogLevel::Trace:
                return this->m_pLogger->set_level(spdlog::level::level_enum::trace);
            case LogLevel::Debug:
                return this->m_pLogger->set_level(spdlog::level::level_enum::debug);
            case LogLevel::Info:
                return this->m_pLogger->set_level(spdlog::level::level_enum::info);
            case LogLevel::Warn:
                return this->m_pLogger->set_level(spdlog::level::level_enum::warn);
            case LogLevel::Error:
                return this->m_pLogger->set_level(spdlog::level::level_enum::err);
            case LogLevel::Critical:
                return this->m_pLogger->set_level(spdlog::level::level_enum::critical);
            case LogLevel::Off:
                return this->m_pLogger->set_level(spdlog::level::level_enum::off);
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

            if (this->m_pLogger != nullptr) {
                this->m_pLogger->flush();
                this->m_pLogger.reset();

                spdlog::shutdown();
            }

            m_isInitialized = false;
        } catch (const spdlog::spdlog_ex &ex) {
            std::cout << "logger stop failed: " << ex.what() << '\n';
        }
    }

    void GlobalLogger::Trace(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->trace(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }

    void GlobalLogger::Debug(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->debug(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }

    void GlobalLogger::Info(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->info(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }

    void GlobalLogger::Warning(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->warn(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }

    void GlobalLogger::Error(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->error(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }

    void GlobalLogger::Critical(const std::string_view &msg, std::source_location &&rLocation) const {
        this->m_pLogger->critical(std::format("[{}:{}] {}", rLocation.file_name(), rLocation.line(), msg.data()));
    }
} // namespace shadow::logger
