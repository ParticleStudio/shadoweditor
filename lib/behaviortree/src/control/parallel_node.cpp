#include "behaviortree/control/parallel_node.h"

#include <algorithm>
#include <cstddef>

namespace behaviortree {
constexpr const char *ParallelNode::THRESHOLD_FAILURE;
constexpr const char *ParallelNode::THRESHOLD_SUCCESS;

ParallelNode::ParallelNode(const std::string &rName): ControlNode::ControlNode(rName, {}),
                                                      m_successThreshold(-1),
                                                      m_failureThreshold(1),
                                                      m_readParameterFromPorts(false) {
    SetRegistrationId("Parallel");
}

ParallelNode::ParallelNode(const std::string &rName, const NodeConfig &rConfig): ControlNode::ControlNode(rName, rConfig),
                                                                                 m_successThreshold(-1),
                                                                                 m_failureThreshold(1),
                                                                                 m_readParameterFromPorts(true) {}

NodeStatus ParallelNode::Tick() {
    if(m_readParameterFromPorts) {
        if(!GetInput(THRESHOLD_SUCCESS, m_successThreshold)) {
            throw util::RuntimeError("Missing parameter [", THRESHOLD_SUCCESS, "] in ParallelNode");
        }

        if(!GetInput(THRESHOLD_FAILURE, m_failureThreshold)) {
            throw util::RuntimeError("Missing parameter [", THRESHOLD_FAILURE, "] in ParallelNode");
        }
    }

    const size_t childrenNum = m_childrenNodeVec.size();

    if(childrenNum < SuccessThreshold()) {
        throw util::LogicError("Number of children is less than threshold. Can never succeed.");
    }

    if(childrenNum < FailureThreshold()) {
        throw util::LogicError("Number of children is less than threshold. Can never fail.");
    }

    SetNodeStatus(NodeStatus::Running);

    size_t skippedCount = 0;

    // Routing the tree according to the sequence node's logic:
    for(size_t i = 0; i < childrenNum; i++) {
        if(m_completedSet.count(i) == 0) {
            TreeNode *pChildNode = m_childrenNodeVec[i];
            NodeStatus const childNodeStatus = pChildNode->ExecuteTick();

            switch(childNodeStatus) {
                case NodeStatus::Skipped: {
                    skippedCount++;
                } break;
                case NodeStatus::Success: {
                    m_completedSet.insert(i);
                    m_successCount++;
                } break;
                case NodeStatus::Failure: {
                    m_completedSet.insert(i);
                    m_failureCount++;
                } break;
                case NodeStatus::Running: {
                    // Still working. Check the next
                } break;
                case NodeStatus::Idle: {
                    throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
                } break;
                default: {
                } break;
            }
        }

        const size_t requiredSuccessCount = SuccessThreshold();

        if(m_successCount >= requiredSuccessCount || (m_successThreshold < 0 && (m_successCount + skippedCount) >= requiredSuccessCount)) {
            Clear();
            ResetChildren();
            return NodeStatus::Success;
        }

        // It fails if it is not possible to succeed anymore or if
        // number of failures are equal to failure_threshold_
        if(((childrenNum - m_failureCount) < requiredSuccessCount) ||
           (m_failureCount == FailureThreshold())) {
            Clear();
            ResetChildren();
            return NodeStatus::Failure;
        }
    }
    // Skip if ALL the nodes have been skipped
    return (skippedCount == childrenNum) ? NodeStatus::Skipped
                                         : NodeStatus::Running;
}

void ParallelNode::Clear() {
    m_completedSet.clear();
    m_successCount = 0;
    m_failureCount = 0;
}

void ParallelNode::Halt() {
    Clear();
    ControlNode::Halt();
}

size_t ParallelNode::SuccessThreshold() const {
    if(m_successThreshold < 0) {
        return static_cast<size_t>(std::max(static_cast<int32_t>(m_childrenNodeVec.size()) + m_successThreshold + 1, 0));
    } else {
        return static_cast<size_t>(m_successThreshold);
    }
}

size_t ParallelNode::FailureThreshold() const {
    if(m_failureThreshold < 0) {
        return static_cast<size_t>(std::max(static_cast<int32_t>(m_childrenNodeVec.size()) + m_failureThreshold + 1, 0));
    } else {
        return static_cast<size_t>(m_failureThreshold);
    }
}

void ParallelNode::SetSuccessThreshold(int threshold) {
    m_successThreshold = threshold;
}

void ParallelNode::SetFailureThreshold(int threshold) {
    m_failureThreshold = threshold;
}

}// namespace behaviortree
