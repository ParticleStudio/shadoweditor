#include "app.h"

#include <map>
#include <numeric>
#include <stack>
#include <thread>

#include "asio.hpp"
#include "common/threadpool.hpp"
#include "logger/logger.h"

namespace server {
App::App(Singleton<App>::Token): m_appState(AppState::UNDEFINED) {
}

/*
* 初始化
* @return ErrCode
*/
ErrCode App::Init() {
    LogInfo("server init", 1);
    this->SetAppState(AppState::INIT);

    return ErrCode::SUCCESS;
}

/*
* 启动
* @return ErrCode
*/
ErrCode App::Start() {
    LogInfo("server start");
    this->SetAppState(AppState::START);

    return ErrCode::SUCCESS;
}

/*
* 运行
* @return ErrCode
*/
ErrCode App::Run() {
    LogInfo("server run");
    this->SetAppState(AppState::RUN);

    for(uint32_t i = 0; i < 1; i++) {
        common::ThreadPool::GetInstance().AddTask([this]() {
            while(this->IsRunning()) {
                //                    int a[] = {1, 2, 3, 4, 5};
                //                    shadow::log::info("a's length is {},n:{}", util::arrayLength(a), i);

                //                shadow::js::Context jsContext = shadow::js::CreateContext();
                //                JSValue jsValue = jsContext.EvalFile("script/main.js");
                //                if(JS_IsException(jsValue)) {
                //                    shadow::log::error("JS_Eval Error jsfile:{}", "script/main.js");
                //                    return ErrCode::FAIL;
                //                }
                //                JS_FreeValue(jsContext.GetContext(), jsValue);
                LogTrace("trace: {}", 0);
                LogDebug("debug: {}", 1);
                LogInfo("info: {}", 2);
                LogWarning("warn: {}", 3);
                LogError("error: {}", 4);
                LogCritical("critical: {}", 5);

                try {
                    asio::io_service pIoService;
                    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 7001);
                    asio::ip::tcp::acceptor acceptor(pIoService, endpoint);
                    LogInfo("TcpServer start with {}:{}", acceptor.local_endpoint().address().to_string(), acceptor.local_endpoint().port());

                    std::shared_ptr<asio::ip::tcp::socket> pSocket = std::make_shared<asio::ip::tcp::socket>(pIoService);
                    acceptor.async_accept(pSocket, std::bind([pIoService](std::shared_ptr<asio::ip::tcp::socket> pSocket, asio::error_code &rErrorCode){
                                              if (rErrorCode) return;

                                              std::shared_ptr<asio::ip::tcp::socket>
                                          }), pSocket);

                    while(true) {
                        asio::ip::tcp::socket socket(pIoService);
                        //                        acceptor.async_accept(pIoService, [](){
                        //
                        //                        });
                        acceptor.accept(socket);
                        LogInfo("client {}:{}", socket.remote_endpoint().address().to_string(), socket.remote_endpoint().port());

                        char data[512];
                        int32_t dataLen = socket.read_some(asio::buffer(data));
                        if(dataLen > 0) {
                            socket.write_some(asio::buffer(std::format("hello {} {}", socket.remote_endpoint().address().to_string(), data)));
                        }
                    }
                } catch(std::exception &err) {
                    LogError(err.what());
                }

//                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        });
    }
    common::ThreadPool::GetInstance().JoinAll();

    return ErrCode::SUCCESS;
}

/*
* 暂停
* @return ErrCode
*/
ErrCode App::Pause() {
    LogInfo("server pause");
    this->SetAppState(AppState::PAUSE);

    return ErrCode::SUCCESS;
}

/*
* 恢复
* @return ErrCode
*/
ErrCode App::Resume() {
    LogInfo("server resume");
    this->SetAppState(AppState::RUN);

    return ErrCode::SUCCESS;
}

/*
* 停止
* @return ErrCode
*/
ErrCode App::Stop() {
    LogInfo("server begin stop");
    this->SetAppState(AppState::STOP);

    return ErrCode::SUCCESS;
}

/*
* 退出
* @return ErrCode
*/
ErrCode App::Exit() noexcept {
    LogInfo("server begin exit");

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
    this->m_appState = rAppState;

    return ErrCode::SUCCESS;
}

bool App::IsRunning() {
    return this->m_appState == AppState::RUN;
}
}// namespace server
