#ifndef BEHAVIORTREE_FALLBACK_NODE_H
#define BEHAVIORTREE_FALLBACK_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The FallbackNode is used to try different strategies,
 * until one succeeds.
 * If any GetChildNode returns RUNNING, previous GetChildrenNode will NOT be ticked again.
 *
 * - If all the GetChildrenNode return FAILURE, this node returns FAILURE.
 *
 * - If a GetChildNode returns RUNNING, this node returns RUNNING.
 *
 * - If a GetChildNode returns SUCCESS, stop the loop and return SUCCESS.
 *
 */
class FallbackNode: public ControlNode {
 public:
    FallbackNode(const std::string &rName, bool makeAsynch = false);

    virtual ~FallbackNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_curChildIdx;
    size_t m_skippedNum{0};
    bool m_asynch{false};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_FALLBACK_NODE_H
