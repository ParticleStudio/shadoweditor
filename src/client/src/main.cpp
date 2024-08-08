#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>

#include "app.h"
#include "common/threadpool.hpp"
#include "logger/logger.h"
#include "net/manager.h"

void SignalHandler(int32_t sig) {
    switch(sig) {
        case SIGINT: {
            try {
//                client::App::GetInstance().Stop();
                common::ThreadPool::GetInstance().Release();
                logger::Release();
            } catch(const std::exception &err) {
                LogCritical(err.what());
            }
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
    signal(SIGSEGV, SignalHandler);
}

int main(int argc, char *argv[]) {
    try {
        std::string logPath = "./logs";
        logger::Init(logPath, logger::LogLevel::Trace, 1024, 1, 32);

    } catch(const std::exception &err) {
        LogCritical(err.what());
    }

    logger::Release();

    return EXIT_SUCCESS;
}
