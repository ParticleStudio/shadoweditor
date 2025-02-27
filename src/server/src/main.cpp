#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>

#include "logger/logger.h"
#include "app.h"

void SignalHandler(int32_t sig) {
    switch(sig) {
        case SIGINT: {
            shadow::App::GetInstance()->Stop();
        } break;
        case SIGTERM: {
            shadow::App::GetInstance()->Stop();
        } break;
        case SIGSEGV: {
            shadow::logger::Critical("segment violation");
        } break;
        default: {
            shadow::logger::Critical(std::format("not catch signal: {}", sig));
        } break;
    }
}

void InitSignalHandler() {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    signal(SIGSEGV, SignalHandler);
}

void Stop() {
    try {
        shadow::logger::Info("222222222222222222222222");
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        shadow::logger::Info("333333333333333333333333");
        shadow::logger::Stop();
        std::cout << "44444444444444444444444444\n";
        exit(EXIT_SUCCESS);
    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;
        exit(EXIT_FAILURE);
    }
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

        shadow::App::GetInstance()->Init();
        shadow::App::GetInstance()->Run();
        shadow::logger::Info("aaaaaaaaaaaaaaaaaaaaaaaaa");
        Stop();

        return EXIT_SUCCESS;

    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;

        Stop();
    } catch(...) {
        std::cout << "unknow exception cathed" << std::endl;

        Stop();
    }
}
