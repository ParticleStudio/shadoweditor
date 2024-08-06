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
                server::App::GetInstance().Stop();
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
        logger::Init(logger::LogLevel::Trace, 1024, 1, 32);

        net::Manager::GetInstance().Init();

        if(argc <= 1) {
            LogError("please input config file");
            return EXIT_FAILURE;
        }

        InitSignalHandler();

        //        shadow::config::Init(argv[1]);

        //        std::locale::global(std::locale(shadow::config::GetString("locale")));

        //        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        common::ThreadPool::GetInstance().Init(10);

        server::App::GetInstance().Init();
        server::App::GetInstance().Start();
        server::App::GetInstance().Run();
        server::App::GetInstance().Exit();

        common::ThreadPool::GetInstance().Release();
    } catch(const std::exception &err) {
        LogCritical(err.what());
    }

    logger::Release();

    return EXIT_SUCCESS;
}
