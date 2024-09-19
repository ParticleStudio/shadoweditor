import common.exception;

#include "behaviortree/decorator/retry_node.h"

namespace behaviortree {
constexpr const char *RetryNode::NUM_ATTEMPTS;

RetryNode::RetryNode(const std::string &rName, int NTries): DecoratorNode(rName, {}),
                                                            m_maxAttempts(NTries),
                                                            m_tryCount(0),
                                                            m_readParameterFromPorts(false) {
    SetRegistrationId("RetryUntilSuccessful");
}

RetryNode::RetryNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig),
                                                                           m_maxAttempts(0),
                                                                           m_tryCount(0),
                                                                           m_readParameterFromPorts(true) {}

void RetryNode::Halt() {
    m_tryCount = 0;
    DecoratorNode::Halt();
}

NodeStatus RetryNode::Tick() {
    if(m_readParameterFromPorts) {
        if(!GetInput(NUM_ATTEMPTS, m_maxAttempts)) {
            throw util::RuntimeError("Missing parameter [", NUM_ATTEMPTS, "] in RetryNode");
        }
    }

    bool doLoop = m_tryCount < m_maxAttempts || m_maxAttempts == -1;

    SetNodeStatus(NodeStatus::Running);

    while(doLoop) {
        NodeStatus preNodeStatus = m_childNode->GetNodeStatus();
        NodeStatus childNodeStatus = m_childNode->ExecuteTick();

        switch(childNodeStatus) {
            case NodeStatus::Success: {
                m_tryCount = 0;
                ResetChildNode();
                return (NodeStatus::Success);
            } break;
            case NodeStatus::Failure: {
                m_tryCount++;
                doLoop = m_tryCount < m_maxAttempts || m_maxAttempts == -1;

                ResetChildNode();

                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && preNodeStatus == NodeStatus::Idle && doLoop) {
                    EmitWakeUpSignal();
                    return NodeStatus::Running;
                }
            } break;
            case NodeStatus::Running: {
                return NodeStatus::Running;
            } break;
            case NodeStatus::Skipped: {
                // to allow it to be skipped again, we must reset the node
                ResetChildNode();
                // the child has been skipped. Slip this too
                return NodeStatus::Skipped;
            } break;
            case NodeStatus::Idle: {
                throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            } break;
            default: {

            } break;
        }
    }

    m_tryCount = 0;
    return NodeStatus::Failure;
}
}// namespace behaviortree
