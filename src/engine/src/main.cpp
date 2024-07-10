#include <iostream>
#include <thread>

#include "behaviortree/behaviortree.h"
#include "behaviortree/bt_factory.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"

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
    ThinkRuntimePort(const std::string &name, const behaviortree::NodeConfig &config)
        : behaviortree::SyncActionNode(name, config) {}

    behaviortree::NodeStatus Tick() override {
        SetOutput("text", "The answer is 42");
        return behaviortree::NodeStatus::SUCCESS;
    }
};

class SayRuntimePort: public behaviortree::SyncActionNode {
 public:
    SayRuntimePort(const std::string &name, const behaviortree::NodeConfig &config)
        : behaviortree::SyncActionNode(name, config) {}

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
    behaviortree::BehaviorTreeFactory factory;

    //-------- register ports that might be defined at runtime --------
    // more verbose way
    behaviortree::PortsList thinkPortsList = {behaviortree::OutputPort<std::string>("text")};
    factory.RegisterBuilder(
            CreateManifest<ThinkRuntimePort>("ThinkRuntimePort", thinkPortsList),
            behaviortree::CreateBuilder<ThinkRuntimePort>()
    );
    // less verbose way
    behaviortree::PortsList sayPortsList = {behaviortree::InputPort<std::string>("message")};
    factory.RegisterNodeType<SayRuntimePort>("SayRuntimePort", sayPortsList);

    factory.RegisterBehaviorTreeFromText(xmlText);
    auto tree = factory.CreateTree("MainTree");
    tree.TickWhileRunning();

    return 0;
}
