#include "behaviortree/decorator/repeat_node.h"

namespace behaviortree {
RepeatNode::RepeatNode(const std::string &rName, int NTries): DecoratorNode(rName, {}),
                                                              m_numCycles(NTries),
                                                              m_repeatCount(0),
                                                              m_readParameterFromPorts(false) {
    SetRegistrationId("Repeat");
}

RepeatNode::RepeatNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig),
                                                                             m_numCycles(0),
                                                                             m_repeatCount(0),
                                                                             m_readParameterFromPorts(true) {}

NodeStatus RepeatNode::Tick() {
    if(m_readParameterFromPorts) {
        if(!GetInput(NUM_CYCLES, m_numCycles)) {
            throw RuntimeError("Missing parameter [", NUM_CYCLES, "] in RepeatNode");
        }
    }

    bool doLoop = m_repeatCount < m_numCycles || m_numCycles == -1;
    if(GetNodeStatus() == NodeStatus::Idle) {
        m_allSkipped = true;
    }
    SetNodeStatus(NodeStatus::Running);

    while(doLoop) {
        NodeStatus const preNodeStatus = m_childNode->GetNodeStatus();
        NodeStatus childNodeStatus = m_childNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_allSkipped &= (childNodeStatus == NodeStatus::Skipped);

        switch(childNodeStatus) {
            case NodeStatus::Success: {
                m_repeatCount++;
                doLoop = m_repeatCount < m_numCycles || m_numCycles == -1;

                ResetChildNode();

                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && preNodeStatus == NodeStatus::Idle && doLoop) {
                    EmitWakeUpSignal();
                    return NodeStatus::Running;
                }
            } break;
            case NodeStatus::Failure: {
                m_repeatCount = 0;
                ResetChildNode();
                return (NodeStatus::Failure);
            } break;
            case NodeStatus::Running: {
                return NodeStatus::Running;
            } break;
            case NodeStatus::Skipped: {
                // to allow it to be skipped again, we must reset the node
                ResetChildNode();
                // the child has been skipped. Skip the decorator too.
                // Don't reset the counter, though !
                return NodeStatus::Skipped;
            } break;
            case NodeStatus::Idle: {
                throw util::LogicError("[", GetNodeName(), "]: A children should not return IDLE");
            } break;
            default: {

            } break;
        }
    }

    m_repeatCount = 0;
    return m_allSkipped ? NodeStatus::Skipped : NodeStatus::Success;
}

void RepeatNode::Halt() {
    m_repeatCount = 0;
    DecoratorNode::Halt();
}
}// namespace behaviortree
