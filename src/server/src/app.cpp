#include "app.h"

#include <map>
#include <numeric>
#include <stack>
#include <thread>

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
    LogInfo("server init");
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

    //        shadow::thread::Pool::instance().addTask("create map", []() {
    //            auto *test_map = new shadow::Map();
    //            test_map->createMap(160, 35, 0);
    //
    //            ErrCode e = shadow::MapPath::instance().aStar(test_map);
    //            shadow::log::info("aStar:{}", (int) e);
    ////            test_map->print_map();
    //        });

    for(uint32_t i = 0; i < 3; i++) {
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
//                std::cout << "00000000" << std::endl;
                LogInfo("1111111111: {}");
                LogDebug("222222222222: {}");
                LogError("33333333333: {}");

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
