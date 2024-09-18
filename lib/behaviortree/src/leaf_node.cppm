export module behaviortree.leaf_node;

import behaviortree.tree_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
export class BEHAVIORTREE_API LeafNode: public TreeNode {
 public:
    LeafNode(const std::string &rName, const NodeConfig &rConfig): TreeNode(rName, rConfig) {}

    virtual ~LeafNode() override = default;

 protected:
};
}// namespace behaviortree

// module behaviortree.leaf_node;
