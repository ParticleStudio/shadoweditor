module;

module behaviortree.test_node;

namespace behaviortree {
TestNode::TestNode(const std::string &rName, const NodeConfig &rConfig, TestNodeConfig testNodeConfig): StatefulActionNode(rName, rConfig),
                                                                                                                      m_testConfig(std::move(testNodeConfig)) {
    SetRegistrationId("TestNode");

    if(m_testConfig.returnStatus == NodeStatus::Idle) {
        throw util::RuntimeError("TestNode can not return IDLE");
    }

    auto prepareScript = [](const std::string &rScript, auto &rExecutor) {
        if(!rScript.empty()) {
            auto result = ParseScript(rScript);
            if(!result) {
                throw util::RuntimeError(result.error());
            }
            rExecutor = result.value();
        }
    };
    prepareScript(m_testConfig.successScript, m_successExecutor);
    prepareScript(m_testConfig.failureScript, m_failureExecutor);
    prepareScript(m_testConfig.postScript, m_postExecutor);
}

NodeStatus behaviortree::TestNode::OnStart() {
    if(m_testConfig.asyncDelay <= std::chrono::milliseconds(0)) {
        return OnCompleted();
    }
    // convert this in an asynchronous operation. Use another thread to count
    // a certain amount of time.
    m_completed = false;
    m_timerQueue.Add(
            std::chrono::milliseconds(m_testConfig.asyncDelay),
            [this](bool aborted) {
                if(!aborted) {
                    m_completed.store(true);
                    this->EmitWakeUpSignal();
                } else {
                    m_completed.store(false);
                }
            }
    );
    return NodeStatus::Running;
}

NodeStatus behaviortree::TestNode::OnRunning() {
    if(m_completed) {
        return OnCompleted();
    }
    return NodeStatus::Running;
}

void TestNode::OnHalted() {
    m_timerQueue.CancelAll();
}

NodeStatus behaviortree::TestNode::OnCompleted() {
    Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};

    auto status = m_testConfig.completeFunc();
    if(status == NodeStatus::Success && m_successExecutor) {
        m_successExecutor(env);
    } else if(status == NodeStatus::Failure && m_failureExecutor) {
        m_failureExecutor(env);
    }
    if(m_postExecutor) {
        m_postExecutor(env);
    }
    return status;
}
}// namespace behaviortree

// module behaviortree.test_node;
// module;
