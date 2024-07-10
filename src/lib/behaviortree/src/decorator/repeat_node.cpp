#include "behaviortree/decorator/repeat_node.h"

namespace behaviortree {
RepeatNode::RepeatNode(const std::string &refName, int NTries): DecoratorNode(refName, {}),
                                                                m_NumCycles(NTries),
                                                                m_RepeatCount(0),
                                                                m_ReadParameterFromPorts(false) {
    SetRegistrationId("Repeat");
}

RepeatNode::RepeatNode(const std::string &refName, const NodeConfig &refConfig): DecoratorNode(refName, refConfig),
                                                                                 m_NumCycles(0),
                                                                                 m_RepeatCount(0),
                                                                                 m_ReadParameterFromPorts(true) {}

NodeStatus RepeatNode::Tick() {
    if(m_ReadParameterFromPorts) {
        if(!GetInput(NUM_CYCLES, m_NumCycles)) {
            throw RuntimeError(
                    "Missing parameter [", NUM_CYCLES, "] in RepeatNode"
            );
        }
    }

    bool doLoop = m_RepeatCount < m_NumCycles || m_NumCycles == -1;
    if(GetNodeStatus() == NodeStatus::IDLE) {
        m_AllSkipped = true;
    }
    SetNodeStatus(NodeStatus::RUNNING);

    while(doLoop) {
        NodeStatus const prevNodeStatus = m_ChildNode->GetNodeStatus();
        NodeStatus childNodeStatus = m_ChildNode->ExecuteTick();

        // switch to RUNNING state as soon as you find an active child
        m_AllSkipped &= (childNodeStatus == NodeStatus::SKIPPED);

        switch(childNodeStatus) {
            case NodeStatus::SUCCESS: {
                m_RepeatCount++;
                doLoop = m_RepeatCount < m_NumCycles || m_NumCycles == -1;

                ResetChild();

                // Return the execution flow if the child is async,
                // to make this interruptable.
                if(RequiresWakeUp() && prevNodeStatus == NodeStatus::IDLE &&
                   doLoop) {
                    EmitWakeUpSignal();
                    return NodeStatus::RUNNING;
                }
            } break;
            case NodeStatus::FAILURE: {
                m_RepeatCount = 0;
                ResetChild();
                return (NodeStatus::FAILURE);
            }
            case NodeStatus::RUNNING: {
                return NodeStatus::RUNNING;
            }
            case NodeStatus::SKIPPED: {
                // to allow it to be skipped again, we must reset the node
                ResetChild();
                // the child has been skipped. Skip the decorator too.
                // Don't reset the counter, though !
                return NodeStatus::SKIPPED;
            }
            case NodeStatus::IDLE: {
                throw LogicError(
                        "[", GetNodeName(),
                        "]: A children should not return IDLE"
                );
            }
        }
    }

    m_RepeatCount = 0;
    return m_AllSkipped ? NodeStatus::SKIPPED : NodeStatus::SUCCESS;
}

void RepeatNode::Halt() {
    m_RepeatCount = 0;
    DecoratorNode::Halt();
}
}// namespace behaviortree
