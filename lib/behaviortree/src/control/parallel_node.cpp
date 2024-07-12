#include "behaviortree/control/parallel_node.h"

#include <algorithm>
#include <cstddef>

namespace behaviortree {
constexpr const char *ParallelNode::THRESHOLD_FAILURE;
constexpr const char *ParallelNode::THRESHOLD_SUCCESS;

ParallelNode::ParallelNode(const std::string &refName): ControlNode::ControlNode(refName, {}),
                                                        m_SuccessThreshold(-1),
                                                        m_FailureThreshold(1),
                                                        m_ReadParameterFromPorts(false) {
    SetRegistrationId("Parallel");
}

ParallelNode::ParallelNode(
        const std::string &refName, const NodeConfig &refConfig
): ControlNode::ControlNode(refName, refConfig),
   m_SuccessThreshold(-1),
   m_FailureThreshold(1),
   m_ReadParameterFromPorts(true) {}

NodeStatus ParallelNode::Tick() {
    if(m_ReadParameterFromPorts) {
        if(!GetInput(THRESHOLD_SUCCESS, m_SuccessThreshold)) {
            throw RuntimeError(
                    "Missing parameter [", THRESHOLD_SUCCESS,
                    "] in ParallelNode"
            );
        }

        if(!GetInput(THRESHOLD_FAILURE, m_FailureThreshold)) {
            throw RuntimeError(
                    "Missing parameter [", THRESHOLD_FAILURE,
                    "] in ParallelNode"
            );
        }
    }

    const size_t childrenCount = m_ChildrenNodesVec.size();

    if(childrenCount < SuccessThreshold()) {
        throw LogicError(
                "Number of children is less than threshold. Can never succeed."
        );
    }

    if(childrenCount < FailureThreshold()) {
        throw LogicError(
                "Number of children is less than threshold. Can never fail."
        );
    }

    SetNodeStatus(NodeStatus::RUNNING);

    size_t skippedCount = 0;

    // Routing the tree according to the sequence node's logic:
    for(size_t i = 0; i < childrenCount; i++) {
        if(m_CompletedList.count(i) == 0) {
            TreeNode *ptrChildNode = m_ChildrenNodesVec[i];
            NodeStatus const childNodeStatus = ptrChildNode->ExecuteTick();

            switch(childNodeStatus) {
                case NodeStatus::SKIPPED: {
                    skippedCount++;
                    break;
                }
                case NodeStatus::SUCCESS: {
                    m_CompletedList.insert(i);
                    m_SuccessCount++;
                    break;
                }
                case NodeStatus::FAILURE: {
                    m_CompletedList.insert(i);
                    m_FailureCount++;
                    break;
                }
                case NodeStatus::RUNNING: {
                    // Still working. Check the next
                    break;
                }
                case NodeStatus::IDLE: {
                    throw LogicError(
                            "[", GetNodeName(),
                            "]: A children should not return IDLE"
                    );
                }
                default: {
                    break;
                }
            }
        }

        const size_t requiredSuccessCount = SuccessThreshold();

        if(m_SuccessCount >= requiredSuccessCount ||
           (m_SuccessThreshold < 0 &&
            (m_SuccessCount + skippedCount) >= requiredSuccessCount)) {
            Clear();
            ResetChildren();
            return NodeStatus::SUCCESS;
        }

        // It fails if it is not possible to succeed anymore or if
        // number of failures are equal to failure_threshold_
        if(((childrenCount - m_FailureCount) < requiredSuccessCount) ||
           (m_FailureCount == FailureThreshold())) {
            Clear();
            ResetChildren();
            return NodeStatus::FAILURE;
        }
    }
    // Skip if ALL the nodes have been skipped
    return (skippedCount == childrenCount) ? NodeStatus::SKIPPED
                                           : NodeStatus::RUNNING;
}

void ParallelNode::Clear() {
    m_CompletedList.clear();
    m_SuccessCount = 0;
    m_FailureCount = 0;
}

void ParallelNode::Halt() {
    Clear();
    ControlNode::Halt();
}

size_t ParallelNode::SuccessThreshold() const {
    if(m_SuccessThreshold < 0) {
        return size_t(std::max(
                int(m_ChildrenNodesVec.size()) + m_SuccessThreshold + 1, 0
        ));
    } else {
        return size_t(m_SuccessThreshold);
    }
}

size_t ParallelNode::FailureThreshold() const {
    if(m_FailureThreshold < 0) {
        return size_t(std::max(
                int(m_ChildrenNodesVec.size()) + m_FailureThreshold + 1, 0
        ));
    } else {
        return size_t(m_FailureThreshold);
    }
}

void ParallelNode::SetSuccessThreshold(int threshold) {
    m_SuccessThreshold = threshold;
}

void ParallelNode::SetFailureThreshold(int threshold) {
    m_FailureThreshold = threshold;
}

}// namespace behaviortree
