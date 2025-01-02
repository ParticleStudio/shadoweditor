#ifndef SHADOW_LOGGER_H
#define SHADOW_LOGGER_H

#include <cstdint>
#include <source_location>
#include <string_view>

#include "logger/common.h"

namespace shadow::logger {
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

LOGGER_API void LogTrace(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogDebug(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogInfo(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogWarning(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogError(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void LogCritical(const std::string_view &, std::source_location &&rLocation = std::source_location::current());

} // namespace shadow::logger

#endif // SHADOW_LOGGER_H
