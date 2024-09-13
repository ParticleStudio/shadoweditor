#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>

#include "logger/logger.h"

import client.app;

#define BUF_SIZE 4096

void SignalHandler(int32_t sig) {
    switch(sig) {
        case SIGINT: {
            try {
                //                client::App::GetInstance().Stop();
                //                common::ThreadPool::GetInstance().Release();
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
        std::string logPath = "./log/client/";
        logger::Init(logPath, logger::LogLevel::Trace, 1024, 1, 32);

        // 打开共享的文件对象
        HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ShareMemory");
        if(hMapFile) {
            LPVOID lpBase = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            while(true){
                // 将共享内存数据拷贝出来
                char szBuffer[BUF_SIZE] = {0};
                strcpy(szBuffer, (char *)lpBase);
                LogInfo("read shared memory data: {}", szBuffer);

                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            }

            // 解除文件映射
            UnmapViewOfFile(lpBase);
            // 关闭内存映射文件对象句柄
            CloseHandle(hMapFile);
        } else {
            // 打开共享内存句柄失败
            LogError("open mapping failed");
        }
    } catch(const std::exception &err) {
        LogCritical(err.what());
    }

    logger::Release();

    return EXIT_SUCCESS;
}
