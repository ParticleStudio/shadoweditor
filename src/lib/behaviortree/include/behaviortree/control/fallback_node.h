#ifndef BEHAVIORTREE_FALLBACK_NODE_H
#define BEHAVIORTREE_FALLBACK_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The FallbackNode is used to try different strategies,
 * until one succeeds.
 * If any GetChild returns RUNNING, previous Children will NOT be ticked again.
 *
 * - If all the Children return FAILURE, this node returns FAILURE.
 *
 * - If a GetChild returns RUNNING, this node returns RUNNING.
 *
 * - If a GetChild returns SUCCESS, stop the loop and return SUCCESS.
 *
 */
class FallbackNode: public ControlNode {
 public:
    FallbackNode(const std::string& refName, bool makeAsynch = false);

    virtual ~FallbackNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_CurrentChildIdx;
    bool m_AllSkipped{true};
    bool m_Asynch{false};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_FALLBACK_NODE_H
