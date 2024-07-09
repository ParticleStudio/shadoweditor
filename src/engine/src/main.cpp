#include <iostream>
#include <thread>

#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "behaviortree/behaviortree.h"
#include "behaviortree/bt_factory.h"

int main(int argc, char **argv) {
    behaviortree::BehaviorTreeFactory factory;

    // Our set of simple Nodes, related to CrossDoor
    CrossDoor cross_door;
    cross_door.registerNodes(factory);

    // Groot2 editor requires a model of your registered Nodes.
    // You don't need to write that by hand, it can be automatically
    // generated using the following command.
    std::string xml_models = behaviortree::writeTreeNodesModelXML(factory);

    factory.registerBehaviorTreeFromText(xml_text);
    auto tree = factory.createTree("MainTree");

    // Connect the Groot2Publisher. This will allow Groot2 to
    // get the tree and poll status updates.
    behaviortree::Groot2Publisher publisher(tree);

    // we want to run this indefinitely
    while(1)
    {
        std::cout << "Start" << std::endl;
        cross_door.reset();
        tree.tickWhileRunning();
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    }
    return 0;
}
