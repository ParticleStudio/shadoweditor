#ifndef JSENGINE_MANAGER_H
#define JSENGINE_MANAGER_H

#include "common/singleton.h"
#include "jsengine/define.h"
#include "quickjs-libc.h"

namespace jsengine {
class COMMON_API Manager;

class Manager final: public common::Singleton<Manager> {
 public:
    explicit Manager(Token);

    ~Manager() final;

    JSContext *Init();

    int32_t EvalBuffer(const void *ptrBuffer, int bufferLen, const char *ptrFileName, int evalFlags);

    int32_t EvalFile(const char *ptrFileName);

 private:
    JSRuntime *m_ptrRuntime;
    JSContext *m_ptrContext;
};
}// namespace jsengine

#endif//JSENGINE_MANAGER_H
