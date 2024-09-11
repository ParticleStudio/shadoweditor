#ifndef BEHAVIORTREE_LEAF_NODE_H
#define BEHAVIORTREE_LEAF_NODE_H

#include "behaviortree/tree_node.h"

namespace behaviortree {
class LeafNode: public TreeNode {
 public:
    LeafNode(const std::string &rName, const NodeConfig &rConfig): TreeNode(rName, rConfig) {}

    virtual ~LeafNode() override = default;

 protected:
};
}// namespace behaviortree

#endif// BEHAVIORTREE_LEAF_NODE_H
