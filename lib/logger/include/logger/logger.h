#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <spdlog/common.h>

#include <cstdarg>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>

#include "common/singleton.h"
#include "spdlog/spdlog.h"

constexpr const char *pMainLoggerName = "mainLogger";
constexpr const char *pErrorLoggerName = "errorLogger";
constexpr const char *pPattern = "%^[%Y-%m-%d %H:%M:%S.%e][%t][%l][%s %!:%#] %v%$";

namespace logger {
enum class LogLevel {
    Trace = spdlog::level::level_enum::trace,
    Debug = spdlog::level::level_enum::debug,
    Info = spdlog::level::level_enum::info,
    Warn = spdlog::level::level_enum::warn,
    Error = spdlog::level::level_enum::err,
    Critical = spdlog::level::level_enum::critical,
    Off = spdlog::level::level_enum::off
};

void Init(LogLevel, int32_t, int32_t, int32_t);

[[maybe_unused]] void SetLogLevel(LogLevel);

void Release();

#define LogTrace(format, ...)                                            \
    {                                                                    \
        auto pLogger = spdlog::get(pMainLoggerName);                     \
        SPDLOG_LOGGER_TRACE(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

#define LogDebug(format, ...)                                            \
    {                                                                    \
        auto pLogger = spdlog::get(pMainLoggerName);                     \
        SPDLOG_LOGGER_DEBUG(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

#define LogInfo(format, ...)                                            \
    {                                                                   \
        auto pLogger = spdlog::get(pMainLoggerName);                    \
        SPDLOG_LOGGER_INFO(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

#define LogWarn(format, ...)                                            \
    {                                                                   \
        auto pLogger = spdlog::get(pMainLoggerName);                    \
        SPDLOG_LOGGER_WARN(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

#define LogError(format, ...)                                            \
    {                                                                    \
        auto pLogger = spdlog::get(pMainLoggerName);                     \
        SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(format), __VA_ARGS__); \
                                                                         \
        pLogger = spdlog::get(pErrorLoggerName);                         \
        SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

#define LogCritical(format, ...)                                            \
    {                                                                       \
        auto pLogger = spdlog::get(pMainLoggerName);                        \
        SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(format), __VA_ARGS__); \
                                                                            \
        pLogger = spdlog::get(pErrorLoggerName);                            \
        SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(format), __VA_ARGS__); \
    }

template<class... Args>
[[maybe_unused]] inline void Trace(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_TRACE(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}

template<class... Args>
[[maybe_unused]] inline void Debug(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_DEBUG(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}

template<class... Args>
[[maybe_unused]] inline void Info(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_INFO(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}

template<class... Args>
[[maybe_unused]] inline void Warn(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_WARN(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}

template<class... Args>
[[maybe_unused]] inline void Error(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);

    pLogger = spdlog::get(pErrorLoggerName);
    SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}

template<class... Args>
[[maybe_unused]] inline void Critical(const char *fmt, Args &&...args) {
    auto pLogger = spdlog::get(pMainLoggerName);
    SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);

    pLogger = spdlog::get(pErrorLoggerName);
    SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(fmt), std::forward<Args>(args)...);
}
}// namespace logger

#endif//LOGGER_LOGGER_H
