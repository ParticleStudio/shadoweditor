#include <filesystem>

#include "behaviortree/behaviortree.h"
#include "behaviortree/factory.cppm"
#include "jsengine/jsmanager.ixx"

static const char *treeText = R"(
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
        return behaviortree::NodeStatus::Success;
    }
};

class SayRuntimePort: public behaviortree::SyncActionNode {
 public:
    SayRuntimePort(const std::string &name, const behaviortree::NodeConfig &config): behaviortree::SyncActionNode(name, config) {}

    // You must override the virtual function tick()
    behaviortree::NodeStatus Tick() override {
        auto msg = GetInput<std::string>("message");
        if(!msg) {
            throw util::RuntimeError("missing required input [message]: ", msg.error());
        }
        std::cout << "Robot says: " << msg.value() << std::endl;
        return behaviortree::NodeStatus::Success;
    }
};

int main(int argc, char **argv) {
    jsengine::JSManager::GetInstance().Init();
    jsengine::JSManager::GetInstance().EvalFile("./script/main.js");

    behaviortree::BehaviorTreeFactory factory;
    //-------- register ports that might be defined at runtime --------
    // more verbose way
    behaviortree::PortMap think_ports = {behaviortree::OutputPort<std::string>("text")};
    factory.RegisterBuilder(CreateManifest<ThinkRuntimePort>("ThinkRuntimePort", think_ports), behaviortree::CreateBuilder<ThinkRuntimePort>());
    // less verbose way
    behaviortree::PortMap say_ports = {behaviortree::InputPort<std::string>("message")};
    factory.RegisterNodeType<SayRuntimePort>("SayRuntimePort", say_ports);
//    factory.RegisterBehaviorTreeFromText(treeText);
    factory.RegisterBehaviorTreeFromText(R"(
        {
            "root":{"id":1,"name":"rootTree"},
            “inPorts”:[{"id":11}]
        }
    )");
    auto tree = factory.CreateTree("behaviortree_0");
    tree.TickWhileRunning();

    return 0;
}
