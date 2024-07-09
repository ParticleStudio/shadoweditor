#ifndef BEHAVIORTREE_LEAF_NODE_H
#define BEHAVIORTREE_LEAF_NODE_H

#include "behaviortree/tree_node.h"

namespace behaviortree {
class LeafNode: public TreeNode {
 public:
    LeafNode(const std::string &refName, const NodeConfig &refConfig): TreeNode(refName, refConfig) {}

    virtual ~LeafNode() override = default;

 protected:
};
}// namespace behaviortree

#endif// BEHAVIORTREE_LEAF_NODE_H
