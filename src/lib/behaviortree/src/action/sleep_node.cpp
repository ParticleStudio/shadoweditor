#include "behaviortree/action/sleep_node.h"

namespace behaviortree {
SleepNode::SleepNode(const std::string& refName, const NodeConfig& refConfig)
    : StatefulActionNode(refName, refConfig), m_TimerWaiting(false) {}

NodeStatus SleepNode::OnStart() {
    uint32_t msec{0};
    if(!GetInput("msec", msec)) {
        throw RuntimeError("Missing parameter [msec] in SleepNode");
    }

    if(msec <= 0) {
        return NodeStatus::SUCCESS;
    }

    SetNodeStatus(NodeStatus::RUNNING);

    m_TimerWaiting = true;

    m_TimerId = m_TimerQueue.Add(std::chrono::milliseconds(msec), [this](bool aborted) {
        std::unique_lock<std::mutex> lk(m_DelayMutex);
        if(!aborted) {
            EmitWakeUpSignal();
        }
        m_TimerWaiting = false;
    });

    return NodeStatus::RUNNING;
}

NodeStatus SleepNode::OnRunning() {
    return m_TimerWaiting ? NodeStatus::RUNNING : NodeStatus::SUCCESS;
}

void SleepNode::OnHalted() {
    m_TimerWaiting = false;
    m_TimerQueue.Cancel(m_TimerId);
}
}// namespace behaviortree
