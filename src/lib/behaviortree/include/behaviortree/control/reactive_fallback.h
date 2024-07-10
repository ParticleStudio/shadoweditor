#ifndef BEHAVIORTREE_REACTIVE_FALLBACK_H
#define BEHAVIORTREE_REACTIVE_FALLBACK_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ReactiveFallback is similar to a ParallelNode.
 * All the Children are ticked from first to last:
 *
 * - If a GetChild returns RUNNING, continue to the next sibling.
 * - If a GetChild returns FAILURE, continue to the next sibling.
 * - If a GetChild returns SUCCESS, stop and return SUCCESS.
 *
 * If all the Children fail, than this node returns FAILURE.
 *
 * IMPORTANT: to work properly, this node should not have more than
 *            a single asynchronous GetChild.
 *
 */
class ReactiveFallback: public ControlNode {
 public:
    ReactiveFallback(const std::string &refName): ControlNode(refName, {}) {}

    /** A ReactiveFallback is not supposed to have more than a single
  * anychronous node; if it does an exception is thrown.
  * You can disabled that check, if you know what you are doing.
  */
    static void EnableException(bool enable);

 private:
    behaviortree::NodeStatus Tick() override;

    void Halt() override;

    int32_t m_RunningChild{-1};
    static bool m_ThrowIfMultipleRunning;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_REACTIVE_FALLBACK_H
