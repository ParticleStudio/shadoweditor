#ifndef LOGGER_MANAGER_H
#define LOGGER_MANAGER_H

#include <source_location>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "common/singleton.hpp"
#include "logger/logger.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"

namespace shadow::logger {
class GlobalLogger final: public shadow::singleton::Singleton<GlobalLogger> {
 public:
    ~GlobalLogger() noexcept override;

 public:
    void Init(const std::string_view &, LogLevel, int32_t, int32_t, int32_t);

    void CreateMainLogger(LogLevel, const std::string_view &, int32_t);

    void CreateErrorLogger(const std::string_view &, int32_t);

    void SetLogLevel(LogLevel) const;

    void Stop();

 public:
    void Trace(const std::string_view &, std::source_location &&) const;
    void Debug(const std::string_view &, std::source_location &&) const;
    void Info(const std::string_view &, std::source_location &&) const;
    void Warning(const std::string_view &, std::source_location &&) const;
    void Error(const std::string_view &, std::source_location &&) const;
    void Critical(const std::string_view &, std::source_location &&) const;

 private:
    std::shared_ptr<spdlog::logger> m_pMainLogger{nullptr};
    std::shared_ptr<spdlog::logger> m_pErrorLogger{nullptr};
    const std::string_view m_pattern{"%^[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v%$"};

    std::mutex m_mutex;
    std::atomic<bool> m_isInitialized{false};
};
} // namespace shadow::logger

#endif // LOGGER_MANAGER_H
