#include "app.h"

#include <map>
#include <numeric>
#include <stack>
#include <thread>

#include "asio.hpp"
#include "common/threadpool.hpp"
#include "logger/logger.h"
#include "net/manager.h"

namespace server {
App::App(Singleton<App>::Token): m_appState(AppState::UNDEFINED) {
}

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

    for(uint32_t i = 0; i < 3; i++) {
        auto taskResult = common::ThreadPool::GetInstance().SubmitTask([this]() {
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

//                net::Manager::GetInstance().NewTcpServer(std::move(asio::ip::tcp::v4()), std::move(std::string("127.0.0.1")), 37001);

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            return std::this_thread::get_id();
        });
//        LogInfo("task result: {}", taskResult.get());
    }

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
    this->m_appState = rAppState;

    return ErrCode::SUCCESS;
}

bool App::IsRunning() {
    return this->m_appState == AppState::RUN;
}
}// namespace server
