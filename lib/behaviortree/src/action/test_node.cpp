#include "behaviortree/action/test_node.h"

namespace behaviortree {
behaviortree::TestNode::TestNode(
        const std::string &refName, const NodeConfig &refConfig,
        TestNodeConfig testNodeConfig
): StatefulActionNode(refName, refConfig),
   m_TestConfig(std::move(testNodeConfig)) {
    SetRegistrationId("TestNode");

    if(m_TestConfig.returnStatus == NodeStatus::Idle) {
        throw RuntimeError("TestNode can not return IDLE");
    }

    auto prepareScript = [](const std::string &refScript, auto &refExecutor) {
        if(!refScript.empty()) {
            auto result = ParseScript(refScript);
            if(!result) {
                throw RuntimeError(result.error());
            }
            refExecutor = result.value();
        }
    };
    prepareScript(m_TestConfig.successScript, m_SuccessExecutor);
    prepareScript(m_TestConfig.failureScript, m_FailureExecutor);
    prepareScript(m_TestConfig.postScript, m_PostExecutor);
}

behaviortree::NodeStatus behaviortree::TestNode::OnStart() {
    if(m_TestConfig.asyncDelay <= std::chrono::milliseconds(0)) {
        return OnCompleted();
    }
    // convert this in an asynchronous operation. Use another thread to count
    // a certain amount of time.
    m_Completed = false;
    m_TimerQueue.Add(
            std::chrono::milliseconds(m_TestConfig.asyncDelay),
            [this](bool aborted) {
                if(!aborted) {
                    m_Completed.store(true);
                    this->EmitWakeUpSignal();
                } else {
                    m_Completed.store(false);
                }
            }
    );
    return NodeStatus::Running;
}

behaviortree::NodeStatus behaviortree::TestNode::OnRunning() {
    if(m_Completed) {
        return OnCompleted();
    }
    return NodeStatus::Running;
}

void behaviortree::TestNode::OnHalted() {
    m_TimerQueue.CancelAll();
}

behaviortree::NodeStatus behaviortree::TestNode::OnCompleted() {
    Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};

    auto status = m_TestConfig.completeFunc();
    if(status == NodeStatus::Success && m_SuccessExecutor) {
        m_SuccessExecutor(env);
    } else if(status == NodeStatus::Failure && m_FailureExecutor) {
        m_FailureExecutor(env);
    }
    if(m_PostExecutor) {
        m_PostExecutor(env);
    }
    return status;
}
}// namespace behaviortree
