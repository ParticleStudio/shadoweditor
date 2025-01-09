#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>

#include "logger/logger.h"
#include "app.h"

void SIGINTHandler(int32_t signal){
    shadow::App::GetInstance()->Stop();
}

void SignalHandler() {
    shadow::App::GetInstance()->Stop();
}

void InitSignalHandler() {
    struct sigaction sa;
    sa.sa_handler = SignalHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, 0);
    sigaction(SIGPIPE, &sa, 0);
    return 0;
}

void Stop() {
    try {
        shadow::logger::Stop();
    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;
    } catch(...) {
        std::cout << "Stop unknow exception cathed" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if(argc <= 1) {
        std::cout << "please input config file\n";
        return EXIT_FAILURE;
    }

    shadow::logger::Init("./log/server/", shadow::logger::LogLevel::Trace, 1024, 1, 32);

    InitSignalHandler();

    try {
        //        shadow::config::Init(argv[1]);

        //        std::locale::global(std::locale(shadow::config::GetString("locale")));

        //        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        shadow::App::GetInstance()->Init();
        shadow::App::GetInstance()->Run();

        Stop();

        return EXIT_SUCCESS;
    } catch(const std::exception &err) {
        std::cout << err.what() << std::endl;

        Stop();
        return EXIT_FAILURE;
    } catch(...) {
        std::cout << "unknow exception cathed\n";

        Stop();
        return EXIT_FAILURE;
    }
}
