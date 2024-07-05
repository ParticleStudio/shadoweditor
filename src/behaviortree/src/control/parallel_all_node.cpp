#include "behaviortree/control/parallel_all_node.h"

#include <algorithm>
#include <cstddef>

namespace behaviortree {
ParallelAllNode::ParallelAllNode(const std::string& refName, const NodeConfig& refConfig)
    : ControlNode::ControlNode(refName, refConfig), m_FailureThreshold(1) {}

NodeStatus ParallelAllNode::Tick() {
    int32_t maxFailures{0};
    if(!GetInput("max_failures", maxFailures)) {
        throw RuntimeError("Missing parameter [max_failures] in ParallelNode");
    }
    const size_t childrenCount = m_ChildrenNodesVec.size();
    SetFailureThreshold(maxFailures);

    size_t skippedCount{0};

    if(childrenCount < m_FailureThreshold) {
        throw LogicError("Number of children is less than threshold. Can never fail.");
    }

    SetNodeStatus(NodeStatus::RUNNING);

    // Routing the tree according to the sequence node's logic:
    for(size_t index = 0; index < childrenCount; index++) {
        TreeNode* ptrChildNode = m_ChildrenNodesVec[index];

        // already completed
        if(m_CompletedList.count(index) != 0) {
            continue;
        }

        NodeStatus const childNodeStatus = ptrChildNode->ExecuteTick();

        switch(childNodeStatus) {
            case NodeStatus::SUCCESS: {
                m_CompletedList.insert(index);
            } break;
            case NodeStatus::FAILURE: {
                m_CompletedList.insert(index);
                m_FailureCount++;
            } break;
            case NodeStatus::RUNNING: {
                // Still working. Check the next
            } break;
            case NodeStatus::SKIPPED: {
                skippedCount++;
            } break;
            case NodeStatus::IDLE: {
                throw LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            }
        }
    }

    if(skippedCount == childrenCount) {
        return NodeStatus::SKIPPED;
    }
    if(skippedCount + m_CompletedList.size() >= childrenCount) {
        // DONE
        HaltChildren();
        m_CompletedList.clear();
        auto const status = (m_FailureCount >= m_FailureThreshold) ? NodeStatus::FAILURE : NodeStatus::SUCCESS;
        m_FailureCount = 0;
        return status;
    }

    // Some children haven't finished, yet.
    return NodeStatus::RUNNING;
}

void ParallelAllNode::Halt() {
    m_CompletedList.clear();
    m_FailureCount = 0;
    ControlNode::Halt();
}

size_t ParallelAllNode::GetFailureThreshold() const {
    return m_FailureThreshold;
}

void ParallelAllNode::SetFailureThreshold(int32_t threshold) {
    if(threshold < 0) {
        m_FailureThreshold = size_t(std::max(int(m_ChildrenNodesVec.size()) + threshold + 1, 0));
    } else {
        m_FailureThreshold = size_t(threshold);
    }
}

}// namespace behaviortree
