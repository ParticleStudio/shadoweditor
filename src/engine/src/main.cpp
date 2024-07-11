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

constexpr const char *jsCode = R"(
    function SayHello(name) {
        console.log("Hello " + name);
    }
)";

int main(int argc, char **argv) {
    JSRuntime *ptrJSRuntime = JS_NewRuntime();
    JSContext *ptrJSContext = JS_NewContext(ptrJSRuntime);

    JSValue jsValue = JS_Eval(ptrJSContext, jsCode, strlen(jsCode), "<SayHello>", 0);
    if (JS_IsException(jsValue)) {
        std::cerr << "Error evaluating script" << std::endl;
        JS_FreeValue(ptrJSContext, jsValue);
        return -1;
    }

    JS_FreeValue(ptrJSContext, jsValue);
    JS_FreeContext(ptrJSContext);
    JS_FreeRuntime(ptrJSRuntime);

    return 0;
}
