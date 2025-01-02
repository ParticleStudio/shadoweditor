#include "logger/logger.h"

#include <ctime>
#include <iostream>

#include "logger/global_logger.h"

namespace shadow::logger {
void Init(const std::string_view &rLogPath, LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    return GlobalLogger::GetInstance()->Init(rLogPath, logLevel, qsize, threadNum, backtraceNum);
}

void Release() {
    std::cout << "logger Release" << std::endl;
    return GlobalLogger::GetInstance()->Release();
}

void SetLogLevel(LogLevel logLevel) {
    return GlobalLogger::GetInstance()->SetLogLevel(logLevel);
}

void LogTrace(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogTrace(msg, std::forward<std::source_location>(rLocation));
}

void LogDebug(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogDebug(msg, std::forward<std::source_location>(rLocation));
}

void LogInfo(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogInfo(msg, std::forward<std::source_location>(rLocation));
}

void LogWarning(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogWarning(msg, std::forward<std::source_location>(rLocation));
}

void LogError(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogError(msg, std::forward<std::source_location>(rLocation));
}

void LogCritical(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->LogCritical(msg, std::forward<std::source_location>(rLocation));
}
} // namespace shadow::logger
