module;

#include <WinSock2.h>
#include <fcntl.h>

#include <ctime>

#include "define.h"

#pragma comment(lib, "ws2_32.lib")

module server.app;

import <array>;
import <cstdint>;
import <map>;
import <mutex>;
import <numeric>;
import <set>;
import <stack>;
import <shared_mutex>;

import threadpool;

#include "logger/logger.h"

namespace server {
/*
* 初始化
* @return ErrCode
*/
ErrCode App::Init() {
    logger::LogInfo("app init");
    this->SetAppState(AppState::INIT);

    return ErrCode::SUCCESS;
}

/*
* 运行
* @return ErrCode
*/
ErrCode App::Run() {
    this->Init();
    this->SetAppState(AppState::RUN);

    // 创建共享内存
    ThreadPool::GetInstance()->DetachTask([this]() {
        int32_t bufSize = 4096;
        // 定义共享数据
        char szBuffer[] = "Hello Shared Memory";

        // 创建共享文件句柄
        HANDLE hMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,// 物理文件句柄
                NULL,                // 默认安全级别
                PAGE_READWRITE,      // 可读可写
                0,                   // 高位文件大小
                bufSize,             // 地位文件大小
                "ShareMemory"        // 共享内存名称
        );

        // 映射缓存区视图, 得到指向共享内存的指针
        LPVOID lpBase = MapViewOfFile(
                hMapFile,           // 共享内存的句柄
                FILE_MAP_ALL_ACCESS,// 可读写许可
                0,
                0,
                bufSize
        );

        // 将数据拷贝到共享内存
        strcpy((char *)lpBase, szBuffer);
        logger::LogInfo(std::format("shared memory:{}", lpBase));

        // 解除文件映射
        UnmapViewOfFile(lpBase);

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));

        // 关闭内存映射文件对象句柄,只要不关闭共享内存的句柄，此进程还在，其他进程就可以读取共享内存。
        CloseHandle(hMapFile);
        logger::LogInfo("shared memory close");

        return 0;
    });

    // 读写锁
    std::shared_mutex sharedMutex;
    ThreadPool::GetInstance()->DetachTask([this, &sharedMutex]() {
        std::unique_lock<std::shared_mutex> lock(sharedMutex);
        logger::LogInfo("unique lock1");
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        logger::LogInfo("unique unlock1");
    });

    ThreadPool::GetInstance()->DetachTask([this, &sharedMutex]() {
        while(true) {
            std::shared_lock<std::shared_mutex> lock(sharedMutex);
            logger::LogInfo("shared lock2");
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            logger::LogInfo("shared unlock2");
        }
    });

    // socket
    std::set<SOCKET> socketSet;
    ThreadPool::GetInstance()->DetachTask([this, &socketSet]() {
        WORD socketVersion = MAKEWORD(2, 2);
        WSAData wsaData;
        if(WSAStartup(socketVersion, &wsaData) != 0) {
            logger::LogError("WSAStartup failed");
            return ErrCode::FAIL;
        }

        SOCKET socketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketListen == INVALID_SOCKET) {
            logger::LogError("create socket failed");
            return ErrCode::FAIL;
        }

        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
        sin.sin_port = htons(8888);
        if(bind(socketListen, (sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
            logger::LogError("socket bind failed");
            return ErrCode::FAIL;
        }

        if(listen(socketListen, 5) == SOCKET_ERROR) {
            logger::LogError("socket listen failed");
            return ErrCode::FAIL;
        }

        while(this->IsRunning()) {
            sockaddr_in clientAddr;
            int32_t clientAddrLen = sizeof(clientAddr);
            SOCKET socketClient = accept(socketListen, (sockaddr *)&clientAddr, &clientAddrLen);
            if(socketClient == INVALID_SOCKET) {
                logger::LogError("socket accept failed");
                return ErrCode::FAIL;
            }

            socketSet.insert(socketClient);

            int32_t ul = 1;
            int32_t ret = ioctlsocket(socketClient, FIONBIO, (unsigned long *)&ul);
            if(ret == SOCKET_ERROR) {
                logger::LogError("socket client ioctlsocket failed");
                return ErrCode::FAIL;
            }

            logger::LogInfo(std::format("socket accept client:{}", socketClient));
        }
        logger::LogInfo("socket listen stop");
    });
    ThreadPool::GetInstance()->DetachTask([this, &socketSet]() {
        while(this->IsRunning()) {
            for(auto socketClient: socketSet) {
                char buf[1024];
                int32_t n = recv(socketClient, buf, sizeof(buf), 0);
                if(n > 0) {
                    logger::LogInfo(std::format("recv:{}", buf));
                    if(strcmp(buf, "s") == 0) {
                        this->SetAppState(AppState::STOP);
                    }
                    send(socketClient, buf, n, 0);
                }
            }
        }
        logger::LogInfo("socket client stop");
    });

    //    int32_t n = 0;
    //    for(uint32_t i = 0; i < 3; i++) {
    //        ThreadPool::GetInstance()->DetachTask([this, &n]() {
    //            while(this->IsRunning()) {
    //                //                int a[] = {1, 2, 3, 4, 5};
    //                //                shadow::log::info("a's length is {},n:{}", util::arrayLength(a), i);
    //                //
    //                //                shadow::js::Context jsContext = shadow::js::CreateContext();
    //                //                JSValue jsValue = jsContext.EvalFile("script/main.js");
    //                //                if(JS_IsException(jsValue)) {
    //                //                    shadow::log::error("JS_Eval Error jsfile:{}", "script/main.js");
    //                //                    return ErrCode::FAIL;
    //                //                }
    //                //                JS_FreeValue(jsContext.GetContext(), jsValue);
    //                //                LogTrace("trace: {}", 0);
    //                //                LogDebug("debug: {}", 1);
    //                //                LogInfo("info: {}", 2);
    //                //                LogWarning("warn: {}", 3);
    //                //                LogError("error: {}", 4);
    //                //                LogCritical("critical: {}", 5);
    //                std::scoped_lock<std::mutex> lock(this->m_mutex);
    //                n++;
    //                LogDebug("n: {}", n);
    //                if(n > 20) this->SetAppState(AppState::STOP);
    //
    //                //                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //            }
    //        });
    //    }
    ThreadPool::GetInstance()->Wait();

    return ErrCode::SUCCESS;
}

/*
* 暂停
* @return ErrCode
*/
ErrCode App::Pause() {
    logger::LogInfo("app pause");
    this->SetAppState(AppState::PAUSE);

    return ErrCode::SUCCESS;
}

/*
* 恢复
* @return ErrCode
*/
ErrCode App::Resume() {
    logger::LogInfo("app resume");
    this->SetAppState(AppState::RUN);

    return ErrCode::SUCCESS;
}

/*
* 停止
* @return ErrCode
*/
ErrCode App::Stop() {
    logger::LogInfo("app begin stop");
    this->SetAppState(AppState::STOP);

    return ErrCode::SUCCESS;
}

/*
* 获取服务器状态
* @return AppState
*/
AppState App::GetAppState() {
    return this->m_appState;
}

/*
* 设置服务器状态
* @return ErrCode
*/
ErrCode App::SetAppState(const AppState &rAppState) {
    logger::LogInfo(std::format("set app state:{}", static_cast<uint32_t>(rAppState)));
    std::scoped_lock<std::mutex> lock(m_mutex);
    this->m_appState = rAppState;

    return ErrCode::SUCCESS;
}

bool App::IsRunning() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    return this->m_appState == AppState::RUN;
}
}// namespace server

// module app
