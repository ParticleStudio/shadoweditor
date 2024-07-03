#ifndef BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
#define BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SequenceWithMemory is used to tick children in an ordered sequence.
 * If any child returns RUNNING, previous children are not ticked again.
 *
 * - If all the children return SUCCESS, this node returns SUCCESS.
 *
 * - If a child returns RUNNING, this node returns RUNNING.
 *   Loop is NOT restarted, the same running child will be ticked again.
 *
 * - If a child returns FAILURE, stop the loop and return FAILURE.
 *   Loop is NOT restarted, the same running child will be ticked again.
 *
 */

class SequenceWithMemory: public ControlNode {
 public:
    SequenceWithMemory(const std::string& refName);

    virtual ~SequenceWithMemory() override = default;

    virtual void Halt() override;

 private:
    size_t m_CurrentChildIdx;
    bool m_AllSkipped{true};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
