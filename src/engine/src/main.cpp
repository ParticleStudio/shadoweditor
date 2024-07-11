#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(__linux__)
#include <malloc.h>
#endif

#include "behaviortree/behaviortree.h"
#include "behaviortree/factory.h"
#include "quickjs-libc.h"
#include "util.h"

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

int main(int argc, char **argv) {
    JSRuntime *ptrRuntime = JS_NewRuntime();
    JSContext *ptrContext = JS_NewContext(ptrRuntime);
//    JS_AddIntrinsicBaseObjects(ptrContext);
//    JS_AddIntrinsicDate(ptrContext);
//    JS_AddIntrinsicEval(ptrContext);
//    JS_AddIntrinsicStringNormalize(ptrContext);
//    JS_AddIntrinsicRegExpCompiler(ptrContext);
//    JS_AddIntrinsicRegExp(ptrContext);
//    JS_AddIntrinsicJSON(ptrContext);
//    JS_AddIntrinsicProxy(ptrContext);
//    JS_AddIntrinsicMapSet(ptrContext);
//    JS_AddIntrinsicTypedArrays(ptrContext);
//    JS_AddIntrinsicPromise(ptrContext);
//    JS_AddIntrinsicBigInt(ptrContext);
//    JS_AddIntrinsicBigFloat(ptrContext);
//    JS_AddIntrinsicBigDecimal(ptrContext);
//    JS_SetModuleLoaderFunc(ptrRuntime, NULL, js_module_loader, NULL);
    js_std_add_helpers(ptrContext, 0, nullptr);

    JSValue jsValue;
    // 读取并执行JavaScript脚本文件
    FILE *fp;
    if(fopen_s(&fp, "./script/main.js", "r") != 0) {
        // 错误处理
        printf("Error opening file.\n");

        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long scriptSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *scriptText = static_cast<char *>(malloc(scriptSize + 1));
    fread(scriptText, scriptSize, 1, fp);
    scriptText[scriptSize] = '\0';// 添加结束符

    fclose(fp);

    jsValue = JS_Eval(ptrContext, scriptText, scriptSize, "main.js", JS_EVAL_TYPE_GLOBAL);
    if(JS_IsException(jsValue)) {
        //        fprintf(stderr, "error evaluating javascript function from main.js: %s\n", JS_ToCString(refContext, jsValue));
        js_std_dump_error(ptrContext);
    }

    JS_FreeContext(ptrContext);
    JS_FreeRuntime(ptrRuntime);
    free(scriptText);

    return 0;
}
