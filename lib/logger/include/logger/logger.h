#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

import <cstdarg>;
import <cstdint>;
import <cstdio>;
import <functional>;
import <iostream>;
import <memory>;
import <mutex>;
import <string_view>;

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "logger/logger_common.h"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

namespace logger {
static constexpr const char *pMainLoggerName = "mainLogger";
static constexpr const char *pErrorLoggerName = "errorLogger";
static constexpr const char *pPattern = "%^[%Y-%m-%d %H:%M:%S.%e][%t][%l][%s %!:%#] %v%$";

enum class LogLevel {
    Trace = spdlog::level::level_enum::trace,
    Debug = spdlog::level::level_enum::debug,
    Info = spdlog::level::level_enum::info,
    Warn = spdlog::level::level_enum::warn,
    Error = spdlog::level::level_enum::err,
    Critical = spdlog::level::level_enum::critical,
    Off = spdlog::level::level_enum::off
};

LOGGER_API void Init(const std::string_view &, LogLevel, int32_t, int32_t, int32_t);

LOGGER_API void Release();

[[maybe_unused]] [[maybe_unused]] LOGGER_API void SetLogLevel(LogLevel);

// todo auto pLogger = spdlog::get(logger::pMainLoggerName); spdlog::get是线程安全的，内部有锁操作，修改为存储pLogger变量，不用每次记录日志都获取pLogger
#define LogTrace(format, ...)                                              \
    {                                                                      \
        auto pLogger = spdlog::get(logger::pMainLoggerName);               \
        SPDLOG_LOGGER_TRACE(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }

#define LogDebug(format, ...)                                              \
    {                                                                      \
        auto pLogger = spdlog::get(logger::pMainLoggerName);               \
        SPDLOG_LOGGER_DEBUG(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }

#define LogInfo(format, ...)                                              \
    {                                                                     \
        auto pLogger = spdlog::get(logger::pMainLoggerName);              \
        SPDLOG_LOGGER_INFO(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }

#define LogWarning(format, ...)                                           \
    {                                                                     \
        auto pLogger = spdlog::get(logger::pMainLoggerName);              \
        SPDLOG_LOGGER_WARN(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }

#define LogError(format, ...)                                              \
    {                                                                      \
        auto pLogger = spdlog::get(logger::pMainLoggerName);               \
        SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
                                                                           \
        pLogger = spdlog::get(logger::pErrorLoggerName);                   \
        SPDLOG_LOGGER_ERROR(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }

#define LogCritical(format, ...)                                              \
    {                                                                         \
        auto pLogger = spdlog::get(logger::pMainLoggerName);                  \
        SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
                                                                              \
        pLogger = spdlog::get(logger::pErrorLoggerName);                      \
        SPDLOG_LOGGER_CRITICAL(pLogger, fmt::runtime(format), ##__VA_ARGS__); \
    }
}// namespace logger

#endif// LOGGER_LOGGER_H
