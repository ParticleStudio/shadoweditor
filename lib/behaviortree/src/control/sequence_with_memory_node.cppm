export module behaviortree.sequence_with_memory_node;

import <string>;

import behaviortree.control_node;

#include "behaviortree/behaviortree_common.h"

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

export class BEHAVIORTREE_API SequenceWithMemory: public ControlNode {
 public:
    SequenceWithMemory(const std::string &rName);

    virtual ~SequenceWithMemory() override = default;

    virtual void Halt() override;

 private:
    size_t m_currentChildIdx;
    bool m_allSkipped{true};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

// module behaviortree.sequence_with_memory_node;
