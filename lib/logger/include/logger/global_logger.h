#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include <source_location>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "common/singleton.hpp"
#include "logger/logger.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"

namespace logger {
class GlobalLogger final: public common::Singleton<GlobalLogger> {
 public:
    ~GlobalLogger();

 public:
    void Init(const std::string_view &, LogLevel, int32_t, int32_t, int32_t);

    void CreateMainLogger(LogLevel, const std::string_view &, int32_t);

    void CreateErrorLogger(const std::string_view &, int32_t);

    void SetLogLevel(LogLevel);

    void Release();

 public:
    void LogTrace(const std::string_view &, std::source_location &&);
    void LogDebug(const std::string_view &, std::source_location &&);
    void LogInfo(const std::string_view &, std::source_location &&);
    void LogWarning(const std::string_view &, std::source_location &&);
    void LogError(const std::string_view &, std::source_location &&);
    void LogCritical(const std::string_view &, std::source_location &&);

 private:
    std::shared_ptr<spdlog::logger> m_pMainLogger{nullptr};
    std::shared_ptr<spdlog::logger> m_pErrorLogger{nullptr};
    const std::string_view m_pattern{"%^[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v%$"};

    std::mutex m_mutex;
    std::atomic<bool> m_isInitialized{false};
};
} // namespace logger

#endif // LOGGER_MANAGER_H
