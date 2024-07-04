#include <behaviortree/behaviortree.h>

#include <iostream>

class MyCondition: public behaviortree::ConditionNode {
 public:
    MyCondition(const std::string& refName);
    ~MyCondition();
    behaviortree::ReturnStatus Tick();
};

MyCondition::MyCondition(const std::string& refName): behaviortree::ConditionNode::ConditionNode(refName) {}

behaviortree::ReturnStatus MyCondition::Tick() {
    std::cout << "The Condition is true" << std::endl;

    return NodeStatus::SUCCESS;
}

class MyAction: public behaviortree::ActionNode {
 public:
    MyAction(const std::string& refName);
    ~MyAction();
    behaviortree::ReturnStatus Tick();
    void Halt();
};

MyAction::MyAction(const std::string& refName): behaviortree::ActionNode::ActionNode(refName) {}

behaviortree::ReturnStatus MyAction::Tick() {
    std::cout << "The Action is doing some operations" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(IsHalted()) {
        return NodeStatus::IDLE;
    }

    std::cout << "The Action is doing some others operations" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(IsHalted()) {
        return NodeStatus::IDLE;
    }

    std::cout << "The Action is doing more operations" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if(IsHalted()) {
        return NodeStatus::IDLE;
    }

    std::cout << "The Action has succeeded" << std::endl;
    return NodeStatus::SUCCESS;
}

void MyAction::Halt() {}

//int main(int argc, char* argv[]) {
//    behaviortree::SequenceNode* seq = new behaviortree::SequenceNode("Sequence");
//    MyCondition* my_con_1 = new MyCondition("Condition");
//    MyAction* my_act_1 = new MyAction("Action");
//    int tick_time_milliseconds = 1000;
//
//    seq->AddChild(my_con_1);
//    seq->AddChild(my_act_1);
//
//    Execute(seq, tick_time_milliseconds);
//
//    return 0;
//}
