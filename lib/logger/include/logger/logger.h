#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <source_location>
#include <string_view>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "logger/common.h"

namespace logger {
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Critical,
    Off,
};

LOGGER_API void Init(const std::string_view &, LogLevel, int32_t, int32_t, int32_t);

LOGGER_API void Release();

LOGGER_API void SetLogLevel(LogLevel);

LOGGER_API void LogTrace(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogDebug(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogInfo(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogWarning(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogError(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogCritical(const std::string_view &msg, std::source_location &&rLocation = std::source_location::current());

}// namespace logger

#endif// LOGGER_LOGGER_H
