#ifndef BEHAVIORTREE_SEQUENCE_NODE_H
#define BEHAVIORTREE_SEQUENCE_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SequenceNode is used to tick GetChildrenNode in an ordered sequence.
 * If any GetChildNode returns RUNNING, previous GetChildrenNode will NOT be ticked again.
 *
 * - If all the GetChildrenNode return SUCCESS, this node returns SUCCESS.
 *
 * - If a GetChildNode returns RUNNING, this node returns RUNNING.
 *   Loop is NOT restarted, the same running GetChildNode will be ticked again.
 *
 * - If a GetChildNode returns FAILURE, stop the loop and return FAILURE.
 *   Restart the loop only if (reset_on_failure == true)
 *
 */

class SequenceNode: public ControlNode {
 public:
    SequenceNode(const std::string &rName, bool rMakeAsync = false);

    virtual ~SequenceNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_currentChildIdx;
    size_t m_skippedNum{0};
    bool m_asynch{false};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SEQUENCE_NODE_H
