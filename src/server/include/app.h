#ifndef SERVER_APP_H
#define SERVER_APP_H

#include "define.h"

import <atomic>;
import <mutex>;

#include "common/singleton.hpp"

namespace server {
class App final: public common::Singleton<App> {
 public:
    ~App() override = default;

    /*
    * 初始化
    * @return ErrCode
    */
    ErrCode Init();

    /*
    * 运行
    * @return ErrCode
    */
    ErrCode Run();

    /*
    * 暂停
    * @return ErrCode
    */
    ErrCode Pause();

    /*
    * 恢复
    * @return ErrCode
    */
    ErrCode Resume();

    /*
    * 停止
    * @return ErrCode
    */
    ErrCode Stop();

    /*
    * 获取状态
    * @return AppState
    */
    AppState GetAppState();

 protected:

 private:
    /*
    * 设置服务器状态
    * @return ErrCode
    */
    ErrCode SetAppState(const AppState &);

    bool IsRunning();

 private:
    std::atomic<AppState> m_appState{AppState::UNDEFINED};
    std::mutex m_mutex;
};
}// namespace server

#endif// SERVER_APP_H
