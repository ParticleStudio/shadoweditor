#include "behaviortree/decorator/timeout_node.h"

namespace behaviortree {
NodeStatus TimeoutNode::Tick() {
    if(m_ReadParameterFromPorts) {
        if(!GetInput("msec", m_Msec)) {
            throw RuntimeError("Missing parameter [msec] in TimeoutNode");
        }
    }

    if(!m_TimeoutStarted) {
        m_TimeoutStarted = true;
        SetNodeStatus(NodeStatus::RUNNING);
        m_ChildHalted = false;

        if(m_Msec > 0) {
            m_TimerId = m_TimerQueue.Add(std::chrono::milliseconds(m_Msec), [this](bool aborted) {
                // Return immediately if the timer was aborted.
                // This function could be invoked during destruction of this object and
                // we don't want to access member variables if not needed.
                if(aborted) {
                    return;
                }
                std::unique_lock<std::mutex> lk(m_TimeoutMutex);
                if(GetChild()->GetNodeStatus() == NodeStatus::RUNNING) {
                    m_ChildHalted = true;
                    HaltChild();
                    EmitWakeUpSignal();
                }
            });
        }
    }

    std::unique_lock<std::mutex> lk(m_TimeoutMutex);

    if(m_ChildHalted) {
        m_TimeoutStarted = false;
        return NodeStatus::FAILURE;
    } else {
        const NodeStatus childNodeStatus = GetChild()->ExecuteTick();
        if(IsStatusCompleted(childNodeStatus)) {
            m_TimeoutStarted = false;
            m_TimeoutMutex.unlock();
            m_TimerQueue.Cancel(m_TimerId);
            m_TimeoutMutex.lock();
            ResetChild();
        }
        return childNodeStatus;
    }
}

void TimeoutNode::Halt() {
    m_TimeoutStarted = false;
    m_TimerQueue.CancelAll();
    DecoratorNode::Halt();
}
}// namespace behaviortree
