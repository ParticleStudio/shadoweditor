#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

#include "behaviortree/behaviortree.h"
#include "behaviortree/factory.h"
#include "quickjs-libc.h"
#include "util.hpp"

// clang-format off
static const char *xmlText = R"(
 <root BTCPP_format="4" >
     <BehaviorTree ID="MainTree">
        <Sequence name="root">
            <ThinkRuntimePort   text="{the_answer}"/>
            <SayRuntimePort     message="{the_answer}" />
        </Sequence>
     </BehaviorTree>
 </root>
 )";
// clang-format on

class ThinkRuntimePort: public behaviortree::SyncActionNode {
 public:
    ThinkRuntimePort(
            const std::string &name, const behaviortree::NodeConfig &config
    ): behaviortree::SyncActionNode(name, config) {}

    behaviortree::NodeStatus Tick() override {
        SetOutput("text", "The answer is 42");
        return behaviortree::NodeStatus::SUCCESS;
    }
};

class SayRuntimePort: public behaviortree::SyncActionNode {
 public:
    SayRuntimePort(
            const std::string &name, const behaviortree::NodeConfig &config
    ): behaviortree::SyncActionNode(name, config) {}

    // You must override the virtual function tick()
    behaviortree::NodeStatus Tick() override {
        auto msg = GetInput<std::string>("message");
        if(!msg) {
            throw behaviortree::RuntimeError(
                    "missing required input [message]: ", msg.error()
            );
        }
        std::cout << "Robot says: " << msg.value() << std::endl;
        return behaviortree::NodeStatus::SUCCESS;
    }
};

static int EvalJSBuffer(JSContext *ctx, const void *buf, int buf_len, const char *filename, int eval_flags) {
    JSValue val;
    int ret;

    if((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
        /* for the modules, we compile then run to be able to set
           import.meta */
        val = JS_Eval(ctx, static_cast<const char *>(buf), buf_len, filename, eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
        if(!JS_IsException(val)) {
            js_module_set_import_meta(ctx, val, true, true);
            val = JS_EvalFunction(ctx, val);
        }
    } else {
        val = JS_Eval(ctx, static_cast<const char *>(buf), buf_len, filename, eval_flags);
    }
    if(JS_IsException(val)) {
        js_std_dump_error(ctx);
        ret = -1;
    } else {
        ret = 0;
    }
    JS_FreeValue(ctx, val);
    return ret;
}

static int32_t EvalJSFile(JSContext *ctx, const char *filename, int module) {
    uint8_t *buf;
    int ret, eval_flags;
    size_t buf_len;

    buf = js_load_file(ctx, &buf_len, filename);
    if(!buf) {
        perror(filename);
        exit(1);
    }

    if(module < 0) {
        module = (util::HasSuffix(filename, ".mjs") || JS_DetectModule((const char *)buf, buf_len));
    }
    if(module)
        eval_flags = JS_EVAL_TYPE_MODULE;
    else
        eval_flags = JS_EVAL_TYPE_GLOBAL;
    ret = EvalJSBuffer(ctx, buf, buf_len, filename, eval_flags);
    js_free(ctx, buf);
    return ret;
}

int main(int argc, char **argv) {
    JSRuntime *ptrRuntime = JS_NewRuntime();
    if(ptrRuntime == nullptr) {
        perror("JS_NewRuntime");
        return -1;
    }
    JSContext *ptrContext = JS_NewContext(ptrRuntime);
    if(ptrContext == nullptr) {
        perror("JS_NewContext");
        return -1;
    }

    //    auto ptrJSGlobalObject =JS_GetGlobalObject(ptrContext);
    //    JS_SetPropertyStr(ptrContext, ptrJSGlobalObject, "exports", ptrJSGlobalObject);

    js_std_init_handlers(ptrRuntime);
    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(ptrRuntime, nullptr, js_module_loader, nullptr);
    js_std_add_helpers(ptrContext, 0, nullptr);
    /* system modules */
    js_init_module_std(ptrContext, "std");
    js_init_module_os(ptrContext, "os");
    /* make 'std' and 'os' visible to non module code */
    const char *str = R"(
                            import * as std from 'std';
                            import * as os from 'os';

                            std.global.std = std;
                            std.global.os = os;
                    )";
    EvalJSBuffer(ptrContext, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);

    js_std_loop(ptrContext);

    int32_t ret = EvalJSFile(ptrContext, "./script/main.js", JS_EVAL_TYPE_MODULE);
    js_std_free_handlers(ptrRuntime);
    JS_FreeContext(ptrContext);
    JS_FreeRuntime(ptrRuntime);

    if(ret != 0) {
        return -1;
    } else {
        return 0;
    }
}
