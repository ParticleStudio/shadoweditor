#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>

#include "logger/logger.h"
#include "app.h"

import shadow.thread.pool;

void Stop() {
    try {
        shadow::App::GetInstance()->Stop();
        shadow::thread::GetGlobalThreadPool()->Release();
        shadow::logger::Release();
    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;
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
            shadow::logger::LogCritical("segment violation");
        } break;
        default: {
            shadow::logger::LogCritical(std::format("not catch signal: {}", sig));
        } break;
    }
}

void InitSignalHandler() {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGSEGV, SignalHandler);
}

int main(int argc, char *argv[]) {
    // if(argc <= 1) {
    //     std::cout << "please input config file" << std::endl;
    //     return EXIT_FAILURE;
    // }

    shadow::logger::Init("./log/server/", shadow::logger::LogLevel::Trace, 1024, 1, 32);

    InitSignalHandler();

    try {
        //        shadow::config::Init(argv[1]);

        //        std::locale::global(std::locale(shadow::config::GetString("locale")));

        //        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        shadow::thread::GetGlobalThreadPool()->Init();

        shadow::App::GetInstance()->Init();
        shadow::App::GetInstance()->Run();

        Stop();

        return EXIT_SUCCESS;

    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;

        Stop();

        return EXIT_FAILURE;
    } catch(...) {
        std::cout << "unknow exception cathed" << std::endl;

        Stop();

        return EXIT_FAILURE;
    }
}
