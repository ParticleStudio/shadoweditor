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

LOGGER_API void Stop();

LOGGER_API void SetLogLevel(LogLevel);

LOGGER_API void Trace(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void Debug(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void Info(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void Warning(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void Error(const std::string_view &, std::source_location &&rLocation = std::source_location::current());
LOGGER_API void Critical(const std::string_view &, std::source_location &&rLocation = std::source_location::current());

} // namespace shadow::logger

#endif // SHADOW_LOGGER_H
