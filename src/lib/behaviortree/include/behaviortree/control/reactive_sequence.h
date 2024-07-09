#ifndef BEHAVIORTREE_REACTIVE_SEQUENCE_H
#define BEHAVIORTREE_REACTIVE_SEQUENCE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ReactiveSequence is similar to a ParallelNode.
 * All the Children are ticked from first to last:
 *
 * - If a GetChild returns RUNNING, halt the remaining siblings in the sequence and return RUNNING.
 * - If a GetChild returns SUCCESS, tick the next sibling.
 * - If a GetChild returns FAILURE, stop and return FAILURE.
 *
 * If all the Children return SUCCESS, this node returns SUCCESS.
 *
 * IMPORTANT: to work properly, this node should not have more than a single
 *            asynchronous GetChild.
 *
 */
class ReactiveSequence: public ControlNode {
 public:
    ReactiveSequence(const std::string& refName): ControlNode(refName, {}) {}

    /** A ReactiveSequence is not supposed to have more than a single
  * anychronous node; if it does an exception is thrown.
  * You can disabled that check, if you know what you are doing.
  */
    static void EnableException(bool enable);

 private:
    behaviortree::NodeStatus Tick() override;

    void Halt() override;

    int m_RunningChild{-1};

    static bool m_ThrowIfMultipleRunning;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_REACTIVE_SEQUENCE_H
