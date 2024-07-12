#include "behaviortree/decorator/subtree_node.h"

namespace behaviortree {
behaviortree::SubTreeNode::SubTreeNode(
        const std::string &name, const NodeConfig &config
): DecoratorNode(name, config) {
    SetRegistrationId("SubTree");
}

behaviortree::PortsList behaviortree::SubTreeNode::ProvidedPorts() {
    auto port = PortInfo(
            PortDirection::INPUT, typeid(bool), GetAnyFromStringFunctor<bool>()
    );
    port.SetDefaultValue(false);
    port.SetDescription(
            "If true, all the ports with the same name "
            "will be remapped"
    );

    return {{"_autoremap", port}};
}

behaviortree::NodeStatus behaviortree::SubTreeNode::Tick() {
    NodeStatus preNodeStatus = GetNodeStatus();
    if(preNodeStatus == NodeStatus::IDLE) {
        SetNodeStatus(NodeStatus::RUNNING);
    }
    const NodeStatus childNodeStatus = m_ChildNode->ExecuteTick();
    if(IsStatusCompleted(childNodeStatus)) {
        ResetNodeStatus();
    }

    return childNodeStatus;
}
}// namespace behaviortree
