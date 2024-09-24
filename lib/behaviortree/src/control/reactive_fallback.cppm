module;

export module behaviortree.reactive_fallback;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ReactiveFallback is similar to a ParallelNode.
 * All the GetChildrenNode are ticked from first to last:
 *
 * - If a GetChildNode returns RUNNING, continue to the next sibling.
 * - If a GetChildNode returns FAILURE, continue to the next sibling.
 * - If a GetChildNode returns SUCCESS, stop and return SUCCESS.
 *
 * If all the GetChildrenNode fail, than this node returns FAILURE.
 *
 * IMPORTANT: to work properly, this node should not have more than
 *            a single asynchronous GetChildNode.
 *
 */
export class BEHAVIORTREE_API ReactiveFallback: public ControlNode {
 public:
    ReactiveFallback(const std::string &rName): ControlNode(rName, {}) {}

    /** A ReactiveFallback is not supposed to have more than a single
  * anychronous node; if it does an exception is thrown.
  * You can disabled that check, if you know what you are doing.
  */
    static void EnableException(bool enable);

 private:
    behaviortree::NodeStatus Tick() override;

    void Halt() override;

    int32_t m_runningChild{-1};
    static bool m_throwIfMultipleRunning;
};

}// namespace behaviortree

// module behaviortree.reactive_fallback;
// module;
