#include "jsengine/manager.h"

#include <cstring>
#include <iostream>

#include "common/util.hpp"

namespace jsengine {
Manager::Manager(common::Singleton<Manager>::Token) {}

Manager::~Manager() {
    js_std_free_handlers(m_ptrRuntime);
    JS_FreeContext(m_ptrContext);
    JS_FreeRuntime(m_ptrRuntime);
}

JSContext *Manager::Init() {
    m_ptrRuntime = JS_NewRuntime();
    if(m_ptrRuntime == nullptr) {
        perror("JS_NewRuntime");
        return nullptr;
    }

    m_ptrContext = JS_NewContext(m_ptrRuntime);
    if(m_ptrContext == nullptr) {
        perror("JS_NewContext");
        return nullptr;
    }

    //    auto ptrJSGlobalObject =JS_GetGlobalObject(ptrContext);
    //    JS_SetPropertyStr(ptrContext, ptrJSGlobalObject, "exports", ptrJSGlobalObject);

    js_std_init_handlers(m_ptrRuntime);
    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(m_ptrRuntime, nullptr, js_module_loader, nullptr);
    js_std_add_helpers(m_ptrContext, 0, nullptr);
    /* system modules */
    (void)js_init_module_std(m_ptrContext, "std");
    (void)js_init_module_os(m_ptrContext, "os");
    /* make 'std' and 'os' visible to non module code */
    const char *ptrJSCode = R"(
        import * as std from 'std';
        import * as os from 'os';

        std.global.std = std;
        std.global.os = os;
    )";
    (void)EvalBuffer(ptrJSCode, strlen(ptrJSCode), "<input>", JS_EVAL_TYPE_MODULE);

    js_std_loop(m_ptrContext);

    return m_ptrContext;
}

int32_t Manager::EvalBuffer(const void *ptrBuffer, int32_t bufferLen, const char *ptrFileName, int evalFlags) {
    JSValue val;
    int ret;

    if((evalFlags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(m_ptrContext, static_cast<const char *>(ptrBuffer), bufferLen, ptrFileName, evalFlags | JS_EVAL_FLAG_COMPILE_ONLY);
        if(!JS_IsException(val)) {
            (void)js_module_set_import_meta(m_ptrContext, val, true, true);
            val = JS_EvalFunction(m_ptrContext, val);
        }
    } else {
        val = JS_Eval(m_ptrContext, static_cast<const char *>(ptrBuffer), bufferLen, ptrFileName, evalFlags);
    }

    if(JS_IsException(val)) {
        js_std_dump_error(m_ptrContext);
        ret = -1;
    } else {
        ret = 0;
    }

    JS_FreeValue(m_ptrContext, val);

    return ret;
}

int32_t Manager::EvalFile(const char *ptrFileName) {
    uint8_t *ptrBuffer;
    int32_t ret, evalFlags;
    size_t bufferLen;
    int32_t module = JS_EVAL_TYPE_MODULE;

    ptrBuffer = js_load_file(m_ptrContext, &bufferLen, ptrFileName);
    if(!ptrBuffer) {
        perror(ptrFileName);
        exit(1);
    }

    if(module < 0) {
        module = (util::HasSuffix(ptrFileName, ".mjs") || JS_DetectModule((const char *)ptrBuffer, bufferLen));
    }
    if(module)
        evalFlags = JS_EVAL_TYPE_MODULE;
    else
        evalFlags = JS_EVAL_TYPE_GLOBAL;
    ret = EvalBuffer(ptrBuffer, bufferLen, ptrFileName, evalFlags);
    js_free(m_ptrContext, ptrBuffer);
    return ret;
}
}// namespace jsengine
