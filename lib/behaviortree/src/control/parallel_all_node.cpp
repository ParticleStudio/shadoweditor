#include "behaviortree/control/parallel_all_node.hpp"

#include <algorithm>
#include <cstddef>

namespace behaviortree {
ParallelAllNode::ParallelAllNode(const std::string &rName, const NodeConfig &rConfig): ControlNode::ControlNode(rName, rConfig), m_failureThreshold(1) {}

NodeStatus ParallelAllNode::Tick() {
    int32_t maxFailures{0};
    if(!GetInput("max_failures", maxFailures)) {
        throw util::RuntimeError("Missing parameter [max_failures] in ParallelNode");
    }
    const size_t childrenNum = m_childrenNodeVec.size();
    SetFailureThreshold(maxFailures);

    size_t skippedCount{0};

    if(childrenNum < m_failureThreshold) {
        throw util::LogicError("Number of children is less than threshold. Can never fail.");
    }

    SetNodeStatus(NodeStatus::Running);

    // Routing the tree according to the sequence node's logic:
    for(size_t index = 0; index < childrenNum; index++) {
        TreeNode *pChildNode = m_childrenNodeVec[index];

        // already completed
        if(m_completedSet.count(index) != 0) {
            continue;
        }

        NodeStatus const childNodeStatus = pChildNode->ExecuteTick();

        switch(childNodeStatus) {
            case NodeStatus::Success: {
                m_completedSet.insert(index);
            } break;
            case NodeStatus::Failure: {
                m_completedSet.insert(index);
                m_failureCount++;
            } break;
            case NodeStatus::Running: {
                // Still working. Check the next
            } break;
            case NodeStatus::Skipped: {
                skippedCount++;
            } break;
            case NodeStatus::Idle: {
                throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            }
        }
    }

    if(skippedCount == childrenNum) {
        return NodeStatus::Skipped;
    }
    if(skippedCount + m_completedSet.size() >= childrenNum) {
        // DONE
        HaltChildren();
        m_completedSet.clear();
        auto const status = (m_failureCount >= m_failureThreshold)
                                    ? NodeStatus::Failure
                                    : NodeStatus::Success;
        m_failureCount = 0;
        return status;
    }

    // Some children haven't finished, yet.
    return NodeStatus::Running;
}

void ParallelAllNode::Halt() {
    m_completedSet.clear();
    m_failureCount = 0;
    ControlNode::Halt();
}

size_t ParallelAllNode::GetFailureThreshold() const {
    return m_failureThreshold;
}

void ParallelAllNode::SetFailureThreshold(int32_t threshold) {
    if(threshold < 0) {
        m_failureThreshold = static_cast<size_t>(std::max(int(m_childrenNodeVec.size()) + threshold + 1, 0));
    } else {
        m_failureThreshold = static_cast<size_t>(threshold);
    }
}

}// namespace behaviortree
