#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>

#include "app.h"
#include "common/threadpool.hpp"
#include "logger/logger.h"

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

        for(int32_t i = 0; i < 10; i++){
            asio::io_service ioService;
            asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 7001);
            asio::ip::tcp::socket socket(ioService);
            socket.connect(endpoint);
            socket.write_some(asio::buffer(std::format("client {}", i)));

            char data[512];
            int32_t dataLen = socket.read_some(asio::buffer(data));
            if(dataLen > 0) {
                LogInfo(data);
            }
        }
    } catch(const std::exception &err) {
        LogCritical(err.what());
    }

    logger::Release();

    return EXIT_SUCCESS;
}
