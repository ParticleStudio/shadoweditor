module;

#include <WinSock2.h>
#include <fcntl.h>

#include "define.h"
#include "logger/logger.h"

#pragma comment(lib, "ws2_32.lib")

module app;

import <array>;
import <cstdint>;
import <map>;
import <mutex>;
import <numeric>;
import <set>;
import <stack>;

import threadpool;

namespace server {
/*
* 初始化
* @return ErrCode
*/
ErrCode App::Init() {
    LogInfo("app init", 1);
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
    std::set<SOCKET> socketSet;

    ThreadPool::GetInstance()->DetachTask([this, &socketSet]() {
        WORD socketVersion = MAKEWORD(2, 2);
        WSAData wsaData;
        if(WSAStartup(socketVersion, &wsaData) != 0) {
            LogError("WSAStartup failed");
            return ErrCode::FAIL;
        }

        SOCKET socketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketListen == INVALID_SOCKET) {
            LogError("create socket failed");
            return ErrCode::FAIL;
        }

        sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
        sin.sin_port = htons(8888);
        if(bind(socketListen, (sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
            LogError("socket bind failed");
            return ErrCode::FAIL;
        }

        if(listen(socketListen, 5) == SOCKET_ERROR) {
            LogError("socket listen failed");
            return ErrCode::FAIL;
        }

        while(this->IsRunning()) {
            sockaddr_in clientAddr;
            int32_t clientAddrLen = sizeof(clientAddr);
            SOCKET socketClient = accept(socketListen, (sockaddr *)&clientAddr, &clientAddrLen);
            if(socketClient == INVALID_SOCKET) {
                LogError("socket accept failed");
                return ErrCode::FAIL;
            }

            socketSet.insert(socketClient);

            int32_t ul = 1;
            int32_t ret = ioctlsocket(socketClient, FIONBIO, (unsigned long *)&ul);
            if(ret == SOCKET_ERROR) {
                LogError("socket client ioctlsocket failed");
                return ErrCode::FAIL;
            }

            LogInfo("socket accept client:{}", socketClient);
        }
        LogInfo("socket listen stop");
    });

    ThreadPool::GetInstance()->DetachTask([this, &socketSet]() {
        while(this->IsRunning()) {
            for(auto socketClient: socketSet) {
                char buf[1024];
                int32_t n = recv(socketClient, buf, sizeof(buf), 0);
                if(n > 0) {
                    LogInfo("recv:{}", buf);
                    if(strcmp(buf, "s") == 0) {
                        this->SetAppState(AppState::STOP);
                    }
                    send(socketClient, buf, n, 0);
                }
            }
        }
        LogInfo("socket client stop");
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
    LogInfo("app pause");
    this->SetAppState(AppState::PAUSE);

    return ErrCode::SUCCESS;
}

/*
* 恢复
* @return ErrCode
*/
ErrCode App::Resume() {
    LogInfo("app resume");
    this->SetAppState(AppState::RUN);

    return ErrCode::SUCCESS;
}

/*
* 停止
* @return ErrCode
*/
ErrCode App::Stop() {
    LogInfo("app begin stop");
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
    LogInfo("set app state:{}", static_cast<uint32_t>(rAppState));
    std::scoped_lock<std::mutex> lock(m_mutex);
    this->m_appState = rAppState;

    return ErrCode::SUCCESS;
}

bool App::IsRunning() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    return this->m_appState == AppState::RUN;
}
}// namespace server
