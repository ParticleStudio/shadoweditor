module behaviortree.timeout_node;

namespace behaviortree {
NodeStatus TimeoutNode::Tick() {
    if(m_readParameterFromPorts) {
        if(!GetInput("msec", m_msec)) {
            throw RuntimeError("Missing parameter [msec] in TimeoutNode");
        }
    }

    if(!m_timeoutStarted) {
        m_timeoutStarted = true;
        SetNodeStatus(NodeStatus::Running);
        m_childHalted = false;

        if(m_msec > 0) {
            m_timerId = m_timerQueue.Add(
                    std::chrono::milliseconds(m_msec),
                    [this](bool aborted) {
                        // Return immediately if the timer was aborted.
                        // This function could be invoked during destruction of this object and
                        // we don't want to access member variables if not needed.
                        if(aborted) {
                            return;
                        }
                        std::unique_lock<std::mutex> lock(m_timeoutMutex);
                        if(GetChildNode()->GetNodeStatus() == NodeStatus::Running) {
                            m_childHalted = true;
                            HaltChildNode();
                            EmitWakeUpSignal();
                        }
                    }
            );
        }
    }

    std::unique_lock<std::mutex> lock(m_timeoutMutex);

    if(m_childHalted) {
        m_timeoutStarted = false;
        return NodeStatus::Failure;
    } else {
        const NodeStatus childNodeStatus = GetChildNode()->ExecuteTick();
        if(IsNodeStatusCompleted(childNodeStatus)) {
            m_timeoutStarted = false;
            m_timeoutMutex.unlock();
            m_timerQueue.Cancel(m_timerId);
            m_timeoutMutex.lock();
            ResetChildNode();
        }
        return childNodeStatus;
    }
}

void TimeoutNode::Halt() {
    m_timeoutStarted = false;
    m_timerQueue.CancelAll();
    DecoratorNode::Halt();
}
}// namespace behaviortree

// module behaviortree.timeout_node;
