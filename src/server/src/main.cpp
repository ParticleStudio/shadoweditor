#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>

#include "app.h"
#include "configure/interface.h"
#include "log/interface.h"
#include "network/interface.h"
#include "threadpool/interface.h"

void SignalHandler(int32_t sig) {
    switch(sig) {
        case SIGINT: {
            try {
                shadow::App::Instance().Stop();
            } catch(const std::exception &err) {
                shadow::log::Critical(err.what());
            }
        } break;
        case SIGSEGV: {
            shadow::log::Critical("segment violation");
        } break;
        default: {

        } break;
    }
}

void InitSignalHandler() {
    signal(SIGINT, SignalHandler);
    signal(SIGSEGV, SignalHandler);
}

int main(int argc, char *argv[]) {
    try {
        ErrCode ret = shadow::log::Init();
        if(ret != ErrCode::SUCCESS) {
            return EXIT_FAILURE;
        }

        if(argc <= 1) {
            shadow::log::Error("please input config file");
            return EXIT_FAILURE;
        }

        InitSignalHandler();

        shadow::config::Init(argv[1]);

        std::locale::global(std::locale(shadow::config::GetString("locale")));

        shadow::log::SetLogLevel(shadow::config::GetInt("loglevel"));

        shadow::threadpool::CreateThread(10);//shadow::config::getInt("threadnum"));

        shadow::App::Instance().Init();
        shadow::App::Instance().Start();
        shadow::App::Instance().Run();
        shadow::App::Instance().Exit();

        shadow::threadpool::Release();
    } catch(const std::exception &err) {
        shadow::log::Critical(err.what());
    }

    shadow::log::Release();

    return EXIT_SUCCESS;
}
