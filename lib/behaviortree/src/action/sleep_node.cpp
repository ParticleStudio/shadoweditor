#include "behaviortree/action/sleep_node.hpp"

namespace behaviortree {
SleepNode::SleepNode(const std::string &rName, const NodeConfig &rConfig): StatefulActionNode(rName, rConfig), m_timerWaiting(false) {}

NodeStatus SleepNode::OnStart() {
    uint32_t msec{0};
    if(!GetInput("msec", msec)) {
        throw util::RuntimeError("Missing parameter [msec] in SleepNode");
    }

    if(msec <= 0) {
        return NodeStatus::Success;
    }

    SetNodeStatus(NodeStatus::Running);

    m_timerWaiting = true;

    m_timerId = m_timerQueue.Add(
            std::chrono::milliseconds(msec),
            [this](bool aborted) {
                std::unique_lock<std::mutex> lock(m_delayMutex);
                if(!aborted) {
                    EmitWakeUpSignal();
                }
                m_timerWaiting = false;
            }
    );

    return NodeStatus::Running;
}

NodeStatus SleepNode::OnRunning() {
    return m_timerWaiting ? NodeStatus::Running : NodeStatus::Success;
}

void SleepNode::OnHalted() {
    m_timerWaiting = false;
    m_timerQueue.Cancel(m_timerId);
}
}// namespace behaviortree
