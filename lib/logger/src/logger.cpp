#include "logger/logger.h"

#include <ctime>
#include <iostream>

#include "logger/global_logger.h"

namespace shadow::logger {
void Init(const std::string_view &rLogPath, LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    return GlobalLogger::GetInstance()->Init(rLogPath, logLevel, qsize, threadNum, backtraceNum);
}

void Stop() {
    std::cout << "logger stop" << std::endl;
    return GlobalLogger::GetInstance()->Stop();
}

void SetLogLevel(LogLevel logLevel) {
    return GlobalLogger::GetInstance()->SetLogLevel(logLevel);
}

void Trace(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Trace(msg, std::forward<std::source_location>(rLocation));
}

void Debug(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Debug(msg, std::forward<std::source_location>(rLocation));
}

void Info(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Info(msg, std::forward<std::source_location>(rLocation));
}

void Warning(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Warning(msg, std::forward<std::source_location>(rLocation));
}

void Error(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Error(msg, std::forward<std::source_location>(rLocation));
}

void Critical(const std::string_view &msg, std::source_location &&rLocation) {
    return GlobalLogger::GetInstance()->Critical(msg, std::forward<std::source_location>(rLocation));
}
} // namespace shadow::logger
