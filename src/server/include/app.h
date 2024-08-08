#ifndef CLIENT_APP_H
#define CLIENT_APP_H

#include <atomic>

#include "common/singleton.h"
#include "define.h"

namespace client {
class App final: public common::Singleton<App> {
 public:
    explicit App(Token);

    ~App() override = default;

    /*
    * 初始化
    * @return ErrCode
    */
    ErrCode Init();

 protected:

 private:
    std::atomic<AppState> m_appState;
};
}// namespace client

#endif// CLIENT_APP_H
