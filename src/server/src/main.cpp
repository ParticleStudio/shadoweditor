#include <ctime> // msvc的bug,使用C++20的module时需要再最前面添加这个include，否则会编译失败[https://developercommunity.visualstudio.com/t/Visual-Studio-cant-find-time-function/1126857]

import common.threadpool;

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <execution>
#include <format>
#include <string>
#include <utility>

#include "app.h"
#include "logger/logger.h"

void Stop() {
    try {
        server::App::GetInstance()->Stop();
        common::GetGlobalThreadPool()->Release();
        logger::Release();
    } catch(const std::exception &err) {
        logger::LogCritical(err.what());
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
            logger::LogCritical("segment violation");
        } break;
        default: {
            logger::LogCritical(std::format("not catch signal: {}", sig));
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
        logger::Init("./log/server/", logger::LogLevel::Trace, 1024, 1, 32);

        if(argc <= 1) {
            logger::LogError("please input config file");
            return EXIT_FAILURE;
        }

        InitSignalHandler();
        //        shadow::config::Init(argv[1]);

        //        std::locale::global(std::locale(shadow::config::GetString("locale")));

        //        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        common::GetGlobalThreadPool()->Init();

        server::App::GetInstance()->Init();
        server::App::GetInstance()->Run();

        Stop();

        return EXIT_SUCCESS;

    } catch(const std::exception &err) {
        logger::LogCritical(err.what());
        Stop();

        return EXIT_FAILURE;
    }
}
