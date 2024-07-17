#include "behaviortree/decorator/subtree_node.h"

namespace behaviortree {
behaviortree::SubTreeNode::SubTreeNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig) {
    SetRegistrationId("SubTree");
}

behaviortree::PortMap behaviortree::SubTreeNode::ProvidedPorts() {
    auto port = PortInfo(PortDirection::Input, typeid(bool), GetAnyFromStringFunctor<bool>());
    port.SetDefaultValue(false);
    port.SetDescription("If true, all the ports with the same name will be remapped");

    return {{"_autoremap", port}};
}

behaviortree::NodeStatus behaviortree::SubTreeNode::Tick() {
    NodeStatus preNodeStatus = GetNodeStatus();
    if(preNodeStatus == NodeStatus::Idle) {
        SetNodeStatus(NodeStatus::Running);
    }
    const NodeStatus childNodeStatus = m_childNode->ExecuteTick();
    if(IsNodeStatusCompleted(childNodeStatus)) {
        ResetNodeStatus();
    }

    return childNodeStatus;
}
}// namespace behaviortree
