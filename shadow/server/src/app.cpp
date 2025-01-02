#include <WinSock2.h>

#include <cstdint>
#include <format>
#include <mutex>
#include <set>
#include <shared_mutex>

#pragma comment(lib, "ws2_32.lib")

#include "define.h"
#include "nlohmann/json.hpp"
#include "logger/logger.h"
#include "app.h"

namespace shadow {
App::~App() noexcept {

}

/*
* 初始化
* @return ErrCode
*/
ErrCode App::Init() {
    shadow::logger::Info("app init");
    this->SetAppState(AppState::INIT);
    this->m_pThreadPool = std::make_unique<shadow::thread::Pool<shadow::thread::tp::none>>(6);

    return ErrCode::SUCCESS;
}

/*
* 运行
* @return ErrCode
*/
ErrCode App::Run() {
    this->SetAppState(AppState::RUN);

    {
        // json
        std::list<nlohmann::json> jsonList;
        const std::string jsonString = R"(
            {
                "pi": 3.141,
                "happy": true,
                "name": "Niels",
                "nothing": null,
                "answer": {
                    "everything": 42
                },
                "list": [1, 0, 2],
                "object": {
                    "currency": "USD",
                    "value": 42.99
                }
            }
        )";

        {
            auto &rJsonObj = jsonList.emplace_back();
            rJsonObj = nlohmann::json::parse(jsonString);
        }
        shadow::logger::Info(std::format("json dump: {}", jsonList.back().dump().data()));
        for(auto &[key, value]: jsonList.back().items()) {
            shadow::logger::Info(std::format("key:{}  value:{}", key.data(), value.dump().data()));
        }
    }

    // 共享内存
    {
        // 创建共享内存
        this->m_pThreadPool->detach_task([]() {
            int32_t bufSize = 4096;
            // 定义共享数据
            char szBuffer[] = "Hello Shared Memory";

            // 创建共享文件句柄
            HANDLE hMapFile = CreateFileMapping(
                    INVALID_HANDLE_VALUE, // 物理文件句柄
                    NULL,                 // 默认安全级别
                    PAGE_READWRITE,       // 可读可写
                    0,                    // 高位文件大小
                    bufSize,              // 地位文件大小
                    "ShareMemory"         // 共享内存名称
            );

            // 映射缓存区视图, 得到指向共享内存的指针
            LPVOID lpBase = MapViewOfFile(
                    hMapFile,            // 共享内存的句柄
                    FILE_MAP_ALL_ACCESS, // 可读写许可
                    0,
                    0,
                    bufSize
            );

            // 将数据拷贝到共享内存
            strcpy((char *)lpBase, szBuffer);
            shadow::logger::Info(std::format("shared memory:{}", lpBase));

            // 解除文件映射
            UnmapViewOfFile(lpBase);

            // 关闭内存映射文件对象句柄,只要不关闭共享内存的句柄，此进程还在，其他进程就可以读取共享内存
            CloseHandle(hMapFile);
            shadow::logger::Info("shared memory close");

            return 0;
        });
    }

    // 读写锁
    std::shared_mutex sharedMutex;
    {
        this->m_pThreadPool->detach_task([this, &sharedMutex]() {
            std::unique_lock<std::shared_mutex> lock(sharedMutex);
            shadow::logger::Debug("unique lock1");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            shadow::logger::Debug("unique unlock1");
        });
        this->m_pThreadPool->detach_task([this, &sharedMutex]() {
            while(this->IsRunning()) {
                std::shared_lock<std::shared_mutex> lock(sharedMutex);
                shadow::logger::Debug("shared lock2");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                shadow::logger::Debug("shared unlock2");
            }
        });
    }

    // socket
    std::set<SOCKET> socketSet{};
    {
        this->m_pThreadPool->detach_task([this, &socketSet]() {
            WORD socketVersion = MAKEWORD(2, 2);
            WSAData wsaData;
            if(WSAStartup(socketVersion, &wsaData) != 0) {
                shadow::logger::Error("WSAStartup failed");
                return;
            }

            SOCKET socketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(socketListen == INVALID_SOCKET) {
                shadow::logger::Error("create socket failed");
                return;
            }

            sockaddr_in sin;
            sin.sin_family = AF_INET;
            sin.sin_addr.S_un.S_addr = INADDR_ANY;
            sin.sin_port = htons(8888);
            if(bind(socketListen, (sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR) {
                shadow::logger::Error("socket bind failed");
                return;
            }

            if(listen(socketListen, 5) == SOCKET_ERROR) {
                shadow::logger::Error("socket listen failed");
                return;
            }

            while(this->IsRunning()) {
                sockaddr_in clientAddr;
                int32_t clientAddrLen = sizeof(clientAddr);
                SOCKET socketClient = accept(socketListen, (sockaddr *)&clientAddr, &clientAddrLen);
                if(socketClient == INVALID_SOCKET) {
                    shadow::logger::Error("socket accept failed");
                    return;
                }

                socketSet.insert(socketClient);

                int32_t ul = 1;
                int32_t ret = ioctlsocket(socketClient, FIONBIO, (unsigned long *)&ul);
                if(ret == SOCKET_ERROR) {
                    shadow::logger::Error("socket client ioctlsocket failed");
                    return;
                }

                shadow::logger::Info(std::format("socket accept client:{}", socketClient));
            }
            shadow::logger::Info("socket listen stop");
        });
        this->m_pThreadPool->detach_task([this, &socketSet]() {
            while(this->IsRunning()) {
                for(auto socketClient: socketSet) {
                    shadow::logger::Info(std::format("socketClient:{}", socketClient));
                    if(!this->IsRunning()) {
                        closesocket(socketClient);
                        socketSet.erase(socketClient);
                        shadow::logger::Info(std::format("socket client close:{}", socketClient));
                        continue;
                    }
                    char buf[1024];
                    int32_t n = recv(socketClient, buf, sizeof(buf), 0);
                    if(n > 0) {
                        shadow::logger::Info(std::format("recv:{}", buf));
                        if(strcmp(buf, "s") == 0) {
                            this->SetAppState(AppState::STOP);
                        }
                        send(socketClient, buf, n, 0);
                    }
                }
            }
            shadow::logger::Info("socket client stop");
        });
    }

    //    int32_t n = 0;
    //    for(uint32_t i = 0; i < 3; i++) {
    //        ThreadPool::GetInstance()->detach_task([this, &n]() {
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
    //                std::scoped_lock<std::mutex> lock(this->m_mutex);
    //                n++;
    //                LogDebug("n: {}", n);
    //                if(n > 20) this->SetAppState(AppState::STOP);
    //
    //                //                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //            }
    //        });
    //    }
    shadow::logger::Info("------------------------");
    this->m_pThreadPool->wait();
    shadow::logger::Info("#######################");

    return ErrCode::SUCCESS;
}

/*
* 暂停
* @return ErrCode
*/
ErrCode App::Pause() {
    shadow::logger::Info("app pause");
    this->SetAppState(AppState::PAUSE);

    return ErrCode::SUCCESS;
}

/*
* 恢复
* @return ErrCode
*/
ErrCode App::Resume() {
    shadow::logger::Info("app resume");
    this->SetAppState(AppState::RUN);

    return ErrCode::SUCCESS;
}

/*
* 停止
* @return ErrCode
*/
ErrCode App::Stop() {
    shadow::logger::Info("app begin stop");
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
    shadow::logger::Info(std::format("set app state:{}", static_cast<uint32_t>(rAppState)));
    std::scoped_lock<std::mutex> lock(m_mutex);
    if(this->m_appState == AppState::STOP) {
        return ErrCode::SUCCESS;
    }

    this->m_appState = rAppState;
    return ErrCode::SUCCESS;
}

bool App::IsRunning() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    return this->m_appState == AppState::RUN;
}
} // namespace shadow
