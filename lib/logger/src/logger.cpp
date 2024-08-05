#include "logger/logger.h"

#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/hourly_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace logger {
inline void CreateLogger(LogLevel logLevel, const char *pPath, int32_t backtraceNum) {
    auto pSinkStdout = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto pSinkHourly = std::make_shared<spdlog::sinks::hourly_file_sink_mt>(pPath, 0, 0);
    spdlog::sinks_init_list sinks{pSinkStdout, pSinkHourly};
    auto pLogger = std::make_shared<spdlog::async_logger>(pMainLoggerName, sinks, spdlog::thread_pool());
    pLogger->set_level(static_cast<spdlog::level::level_enum>(logLevel));
    pLogger->set_pattern(pPattern);
    pLogger->enable_backtrace(backtraceNum);

    spdlog::register_logger(pLogger);
}

inline void CreateLoggerError(const char *pPath, int32_t backtraceNum) {
    auto pLogger = spdlog::hourly_logger_mt(pErrorLoggerName, pPath, false, 0);
    pLogger->set_level(spdlog::level::level_enum::err);
    pLogger->set_pattern(pPattern);
    pLogger->enable_backtrace(backtraceNum);
}

void Init(LogLevel logLevel, int32_t qsize, int32_t threadNum, int32_t backtraceNum) {
    try {
        spdlog::init_thread_pool(qsize, threadNum);
        CreateLogger(logLevel, "logs/main.log", backtraceNum);
        CreateLoggerError("logs/error.log", backtraceNum);
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log init failed:" << ex.what() << std::endl;

        Release();
    }
}

void Release() {
    try {
        spdlog::drop_all();
        spdlog::shutdown();
    } catch(const spdlog::spdlog_ex &ex) {
        std::cout << "log release failed:" << ex.what() << std::endl;
    }
}

[[maybe_unused]] void SetLogLevel(LogLevel logLevel) {
    spdlog::get(pMainLoggerName)->set_level(static_cast<spdlog::level::level_enum>(logLevel));
}
}// namespace logger