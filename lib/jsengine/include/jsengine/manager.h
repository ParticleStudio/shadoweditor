#ifndef JSENGINE_MANAGER_H
#define JSENGINE_MANAGER_H

#ifdef SHARED_LIB
#    ifdef WIN32
#        ifdef DLLEXPORT
#            define JSENGINE_API __declspec(dllexport)
#        else
#            define JSENGINE_API __declspec(dllimport)
#        endif// !DLLEXPORT
#    else
#        define JSENGINE_API
#    endif// !WIN32
#else
#    define JSENGINE_API
#endif// !SHARED_LIB

#include "common/singleton.h"
#include "quickjs-libc.h"

namespace jsengine {
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
