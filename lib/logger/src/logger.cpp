#include "logger/logger.h"

#include "common/string.hpp"
#include "logger/logger_manager.h"

namespace logger {
void Init(const std::string_view &rLogPath, LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    return LoggerManager::GetInstance()->Init(rLogPath, logLevel, qsize, threadNum, backtraceNum);
}

void Release() {
    return LoggerManager::GetInstance()->Release();
}

void SetLogLevel(LogLevel logLevel) {
    return LoggerManager::GetInstance()->SetLogLevel(logLevel);
}

void LogTrace(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogTrace(msg, std::forward<std::source_location>(rLocation));
}

void LogDebug(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogDebug(msg, std::forward<std::source_location>(rLocation));
}

void LogInfo(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogInfo(msg, std::forward<std::source_location>(rLocation));
}

void LogWarning(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogWarning(msg, std::forward<std::source_location>(rLocation));
}

void LogError(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogError(msg, std::forward<std::source_location>(rLocation));
}

void LogCritical(const std::string_view &msg, std::source_location &&rLocation) {
    return LoggerManager::GetInstance()->LogCritical(msg, std::forward<std::source_location>(rLocation));
}
}// namespace logger