module behaviortree.delay_node;

namespace behaviortree {
DelayNode::DelayNode(const std::string &rName, uint32_t milliseconds): DecoratorNode(rName, {}),
                                                                       m_delayStarted(false),
                                                                       m_delayAborted(false),
                                                                       m_msec(milliseconds),
                                                                       m_readParameterFromPorts(false) {
    SetRegistrationId("Delay");
}

DelayNode::DelayNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig),
                                                                           m_delayStarted(false),
                                                                           m_delayAborted(false),
                                                                           m_msec(0),
                                                                           m_readParameterFromPorts(true) {}

void DelayNode::Halt() {
    m_delayStarted = false;
    m_timerQueue.CancelAll();
    DecoratorNode::Halt();
}

NodeStatus DelayNode::Tick() {
    if(m_readParameterFromPorts) {
        if(!GetInput("delay_msec", m_msec)) {
            throw RuntimeError("Missing parameter [delay_msec] in DelayNode");
        }
    }

    if(!m_delayStarted) {
        m_delayComplete = false;
        m_delayAborted = false;
        m_delayStarted = true;
        SetNodeStatus(NodeStatus::Running);

        m_timerId = m_timerQueue.Add(
                std::chrono::milliseconds(m_msec),
                [this](bool aborted) {
                    std::unique_lock<std::mutex> lock(m_delayMutex);
                    m_delayComplete = (!aborted);
                    if(!aborted) {
                        EmitWakeUpSignal();
                    }
                }
        );
    }

    std::unique_lock<std::mutex> lock(m_delayMutex);

    if(m_delayAborted) {
        m_delayAborted = false;
        m_delayAborted = false;
        return NodeStatus::Failure;
    } else if(m_delayComplete) {
        const NodeStatus childNodeStatus = GetChildNode()->ExecuteTick();
        if(IsNodeStatusCompleted(childNodeStatus)) {
            m_delayStarted = false;
            m_delayAborted = false;
            ResetChildNode();
        }
        return childNodeStatus;
    } else {
        return NodeStatus::Running;
    }
}
}// namespace behaviortree

// module behaviortree.delay_node;
