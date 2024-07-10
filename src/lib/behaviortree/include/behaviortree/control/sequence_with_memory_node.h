#ifndef BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
#define BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SequenceWithMemory is used to tick Children in an ordered sequence.
 * If any GetChild returns RUNNING, previous Children are not ticked again.
 *
 * - If all the Children return SUCCESS, this node returns SUCCESS.
 *
 * - If a GetChild returns RUNNING, this node returns RUNNING.
 *   Loop is NOT restarted, the same running GetChild will be ticked again.
 *
 * - If a GetChild returns FAILURE, stop the loop and return FAILURE.
 *   Loop is NOT restarted, the same running GetChild will be ticked again.
 *
 */

class SequenceWithMemory: public ControlNode {
 public:
    SequenceWithMemory(const std::string &refName);

    virtual ~SequenceWithMemory() override = default;

    virtual void Halt() override;

 private:
    size_t m_CurrentChildIdx;
    bool m_AllSkipped{true};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
