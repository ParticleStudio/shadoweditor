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
#include "jsengine/jsengine.h"

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
    jsengine::JSEngine::GetInstance().Init();
    jsengine::JSEngine::GetInstance().EvalFile("./script/main.js");

    return 0
}
