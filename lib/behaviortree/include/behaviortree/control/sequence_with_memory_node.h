#ifndef BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
#define BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SequenceWithMemory is used to tick GetChildrenNode in an ordered sequence.
 * If any GetChildNode returns RUNNING, previous GetChildrenNode are not ticked again.
 *
 * - If all the GetChildrenNode return SUCCESS, this node returns SUCCESS.
 *
 * - If a GetChildNode returns RUNNING, this node returns RUNNING.
 *   Loop is NOT restarted, the same running GetChildNode will be ticked again.
 *
 * - If a GetChildNode returns FAILURE, stop the loop and return FAILURE.
 *   Loop is NOT restarted, the same running GetChildNode will be ticked again.
 *
 */

class SequenceWithMemory: public ControlNode {
 public:
    SequenceWithMemory(const std::string &rName);

    virtual ~SequenceWithMemory() override = default;

    virtual void Halt() override;

 private:
    size_t m_currentChildIdx;
    size_t m_skippedNum{0};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SEQUENCE_WITH_MEMORY_NODE_H
