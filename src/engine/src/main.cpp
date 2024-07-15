#include <filesystem>

#include "behaviortree/behaviortree.h"
#include "behaviortree/factory.h"
#include "jsengine/manager.h"

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
    ThinkRuntimePort(const std::string &name, const behaviortree::NodeConfig &config): behaviortree::SyncActionNode(name, config) {}

    behaviortree::NodeStatus Tick() override {
        SetOutput("text", "The answer is 42");
        return behaviortree::NodeStatus::SUCCESS;
    }
};

class SayRuntimePort: public behaviortree::SyncActionNode {
 public:
    SayRuntimePort(const std::string &name, const behaviortree::NodeConfig &config): behaviortree::SyncActionNode(name, config) {}

    // You must override the virtual function tick()
    behaviortree::NodeStatus Tick() override {
        auto msg = GetInput<std::string>("message");
        if(!msg) {
            throw behaviortree::RuntimeError("missing required input [message]: ", msg.error());
        }
        std::cout << "Robot says: " << msg.value() << std::endl;
        return behaviortree::NodeStatus::SUCCESS;
    }
};

int main(int argc, char **argv) {
    jsengine::Manager::GetInstance().Init();
    jsengine::Manager::GetInstance().EvalFile("./script/main.js");

    return 0;
}
