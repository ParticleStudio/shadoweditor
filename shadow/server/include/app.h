#ifndef SHADOW_SERVER_APP_H
#define SHADOW_SERVER_APP_H

#include <atomic>
#include <mutex>

#include "common/singleton.hpp"
#include "define.h"

namespace shadow {
class App final: public shadow::singleton::Singleton<App> {
 public:
    ~App() noexcept override;

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
}// namespace shadow

#endif// SHADOW_SERVER_APP_H
