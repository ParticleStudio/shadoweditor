#include "behaviortree/decorator/delay_node.h"

namespace behaviortree {
DelayNode::DelayNode(const std::string& refName, uint32_t milliseconds)
    : DecoratorNode(refName, {}), m_DelayStarted(false), m_DelayAborted(false), m_Msec(milliseconds), m_ReadParameterFromPorts(false) {
    SetRegistrationId("Delay");
}

DelayNode::DelayNode(const std::string& refName, const NodeConfig& refConfig)
    : DecoratorNode(refName, refConfig), m_DelayStarted(false), m_DelayAborted(false), m_Msec(0), m_ReadParameterFromPorts(true) {}

void DelayNode::Halt() {
    m_DelayStarted = false;
    m_TimerQueue.CancelAll();
    DecoratorNode::Halt();
}

NodeStatus DelayNode::Tick() {
    if(m_ReadParameterFromPorts) {
        if(!GetInput("delay_msec", m_Msec)) {
            throw RuntimeError("Missing parameter [delay_msec] in DelayNode");
        }
    }

    if(!m_DelayStarted) {
        m_DelayComplete = false;
        m_DelayAborted = false;
        m_DelayStarted = true;
        SetNodeStatus(NodeStatus::RUNNING);

        m_TimerId = m_TimerQueue.Add(std::chrono::milliseconds(m_Msec), [this](bool aborted) {
            std::unique_lock<std::mutex> lk(m_DelayMutex);
            m_DelayComplete = (!aborted);
            if(!aborted) {
                EmitWakeUpSignal();
            }
        });
    }

    std::unique_lock<std::mutex> lk(m_DelayMutex);

    if(m_DelayAborted) {
        m_DelayAborted = false;
        m_DelayAborted = false;
        return NodeStatus::FAILURE;
    } else if(m_DelayComplete) {
        const NodeStatus childNodeStatus = GetChild()->ExecuteTick();
        if(IsStatusCompleted(childNodeStatus)) {
            m_DelayStarted = false;
            m_DelayAborted = false;
            ResetChild();
        }
        return childNodeStatus;
    } else {
        return NodeStatus::RUNNING;
    }
}
}// namespace behaviortree
