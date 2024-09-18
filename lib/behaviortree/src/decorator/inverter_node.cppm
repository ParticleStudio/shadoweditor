export module behaviortree.inverter_node;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The InverterNode returns SUCCESS if GetChildNode fails
 * of FAILURE is GetChildNode succeeds.
 * RUNNING status is propagated
 */
export class BEHAVIORTREE_API InverterNode: public DecoratorNode {
 public:
    InverterNode(const std::string &rName);

    virtual ~InverterNode() override = default;

 private:
    virtual behaviortree::NodeStatus Tick() override;
};
}// namespace behaviortree

// module behaviortree.inverter_node;
