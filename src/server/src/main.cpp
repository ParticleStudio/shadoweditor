#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <execution>

#include "app.h"
#include "threadpool.h"
#include "logger/logger.h"

void Stop() {
    try {
        server::App::GetInstance().Stop();
        ThreadPool::GetInstance().Release();
        logger::Release();
    } catch(const std::exception &err) {
        LogCritical(err.what());
    }
}

void SignalHandler(int32_t sig) {
    switch(sig) {
        case SIGINT: {
            Stop();
        } break;
        case SIGTERM: {
            Stop();
        } break;
        case SIGSEGV: {
            LogCritical("segment violation");
        } break;
        default: {
            LogCritical("not catch signal: {}", sig);
        } break;
    }
}

void InitSignalHandler() {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGSEGV, SignalHandler);
}

int main(int argc, char *argv[]) {
    try {
        std::string logPath = "./logs";
        logger::Init(logPath, logger::LogLevel::Trace, 1024, 1, 32);

        if(argc <= 1) {
            LogError("please input config file");
            return EXIT_FAILURE;
        }

        InitSignalHandler();
        //        shadow::config::Init(argv[1]);

        //        std::locale::global(std::locale(shadow::config::GetString("locale")));

        //        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        ThreadPool::GetInstance().Init();

        server::App::GetInstance().Run();

        ThreadPool::GetInstance().Release();
    } catch(const std::exception &err) {
        LogCritical(err.what());
    }

    logger::Release();

    return EXIT_SUCCESS;
}
