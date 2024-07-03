#include "behaviortree/control_node.h"

namespace behaviortree {
ControlNode::ControlNode(const std::string& name, const NodeConfig& config)
    : TreeNode::TreeNode(name, config) {}

void ControlNode::addChild(TreeNode* child) {
    children_nodes_.push_back(child);
}

size_t ControlNode::childrenCount() const {
    return children_nodes_.size();
}

void ControlNode::halt() {
    resetChildren();
    resetStatus();// might be redundant
}

void ControlNode::resetChildren() {
    for(auto child: children_nodes_) {
        if(child->status() == NodeStatus::RUNNING) {
            child->haltNode();
        }
        child->resetStatus();
    }
}

const std::vector<TreeNode*>& ControlNode::children() const {
    return children_nodes_;
}

void ControlNode::haltChild(size_t i) {
    auto child = children_nodes_[i];
    if(child->status() == NodeStatus::RUNNING) {
        child->haltNode();
    }
    child->resetStatus();
}

void ControlNode::haltChildren() {
    for(size_t i = 0; i < children_nodes_.size(); i++) {
        haltChild(i);
    }
}

void ControlNode::haltChildren(size_t first) {
    for(size_t i = first; i < children_nodes_.size(); i++) {
        haltChild(i);
    }
}

}// namespace behaviortree
