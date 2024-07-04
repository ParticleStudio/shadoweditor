#ifndef BEHAVIORTREE_SEQUENCE_NODE_H
#define BEHAVIORTREE_SEQUENCE_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SequenceNode is used to tick Children in an ordered sequence.
 * If any Child returns RUNNING, previous Children will NOT be ticked again.
 *
 * - If all the Children return SUCCESS, this node returns SUCCESS.
 *
 * - If a Child returns RUNNING, this node returns RUNNING.
 *   Loop is NOT restarted, the same running Child will be ticked again.
 *
 * - If a Child returns FAILURE, stop the loop and return FAILURE.
 *   Restart the loop only if (reset_on_failure == true)
 *
 */

class SequenceNode: public ControlNode {
 public:
    SequenceNode(const std::string& refName, bool makeAsync = false);

    virtual ~SequenceNode() override = default;

    virtual void Halt() override;

 private:
    size_t m_CurrentChildIdx;
    bool m_AllSkipped{true};
    bool m_Asynch{false};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SEQUENCE_NODE_H
