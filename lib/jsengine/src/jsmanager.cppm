module;

#include "jsengine/common.h"
#include "quickjs-libc.h"

export module shadow.jsengine.manager;

namespace shadow::jsengine {
export class JSENGINE_API JSManager {
 public:
    JSManager() = default;

    virtual ~JSManager();

    JSContext *Init();

    int32_t EvalBuffer(const void *ptrBuffer, int bufferLen, const char *ptrFileName, int evalFlags);

    int32_t EvalFile(const char *ptrFileName);

 private:
    JSRuntime *m_ptrRuntime;
    JSContext *m_ptrContext;
};
} // namespace shadow::jsengine

// module shadow.jsengine.manager;
// module;
