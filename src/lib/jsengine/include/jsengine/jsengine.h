#ifndef JSENGINE_JSENGINE_H
#define JSENGINE_JSENGINE_H

# ifdef SHARED_LIB
#   ifdef WIN32
#      ifdef DLLEXPORT
#        define JSENGINE_API __declspec(dllexport)
#      else
#        define JSENGINE_API __declspec(dllimport)
#      endif // !DLLEXPORT
#   else
#     define JSENGINE_API
#   endif // !WIN32
# else
#    define JSENGINE_API
# endif // !SHARED_LIB

#include "common/singleton.hpp"
#include "quickjs-libc.h"

namespace jsengine {
class JSENGINE_API JSEngine final: public common::Singleton<JSEngine> {
 public:
    explicit JSEngine(Token);

    ~JSEngine() final;

    JSContext *Init();

    int32_t EvalBuffer(const void *ptrBuffer, int bufferLen, const char *ptrFileName, int evalFlags);

    int32_t EvalFile(const char *ptrFileName);

 private:
    JSRuntime *m_ptrRuntime;
    JSContext *m_ptrContext;
};
}// namespace jsengine

#endif//JSENGINE_JSENGINE_H
