#include "behaviortree/decorator/retry_node.h"

namespace behaviortree {
constexpr const char *RetryNode::NUM_ATTEMPTS;

RetryNode::RetryNode(const std::string &refName, int NTries): DecoratorNode(refName, {}),
                                                              m_MaxAttempts(NTries),
                                                              m_TryCount(0),
                                                              m_ReadParameterFromPorts(false) {
    SetRegistrationId("RetryUntilSuccessful");
}

RetryNode::RetryNode(const std::string &refName, const NodeConfig &refConfig): DecoratorNode(refName, refConfig),
                                                                               m_MaxAttempts(0),
                                                                               m_TryCount(0),
                                                                               m_ReadParameterFromPorts(true) {}

void RetryNode::Halt() {
    m_TryCount = 0;
    DecoratorNode::Halt();
}

NodeStatus RetryNode::Tick() {
    if(m_ReadParameterFromPorts) {
        if(!GetInput(NUM_ATTEMPTS, m_MaxAttempts)) {
            throw RuntimeError(
                    "Missing parameter [", NUM_ATTEMPTS, "] in RetryNode"
            );
        }
    }

    bool doLoop = m_TryCount < m_MaxAttempts || m_MaxAttempts == -1;

    if(GetNodeStatus() == NodeStatus::Idle) {
        m_AllSkipped = true;
    }
    SetNodeStatus(NodeStatus::Running);

    while(doLoop) {
        NodeStatus prevNodeStatus = m_ChildNode->GetNodeStatus();
        NodeStatus childNodeStatus = m_ChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_AllSkipped &= (childNodeStatus == NodeStatus::Skipped);

        switch(childNodeStatus) {
            case NodeStatus::Success: {
                m_TryCount = 0;
                ResetChild();
                return (NodeStatus::Success);
            }
            case NodeStatus::Failure: {
                m_TryCount++;
                doLoop = m_TryCount < m_MaxAttempts || m_MaxAttempts == -1;

                ResetChild();

                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && prevNodeStatus == NodeStatus::Idle &&
                   doLoop) {
                    EmitWakeUpSignal();
                    return NodeStatus::Running;
                }
            } break;
            case NodeStatus::Running: {
                return NodeStatus::Running;
            }
            case NodeStatus::Skipped: {
                // to allow it to be skipped again, we must reset the node
                ResetChild();
                // the child has been skipped. Slip this too
                return NodeStatus::Skipped;
            }
            case NodeStatus::Idle: {
                throw LogicError(
                        "[", GetNodeName(),
                        "]: A children should not return IDLE"
                );
            }
        }
    }

    m_TryCount = 0;
    return m_AllSkipped ? NodeStatus::Skipped : NodeStatus::Failure;
}
}// namespace behaviortree
