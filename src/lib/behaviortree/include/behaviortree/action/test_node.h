#ifndef BEHAVIORTREE_TEST_NODE_H
#define BEHAVIORTREE_TEST_NODE_H

#include "behaviortree/action_node.h"
#include "behaviortree/scripting/script_parser.hpp"
#include "behaviortree/util/timer_queue.h"

namespace behaviortree {

struct TestNodeConfig {
    /// status to return when the action is completed.
    NodeStatus returnStatus{NodeStatus::SUCCESS};

    /// script to execute when complete_func() returns SUCCESS
    std::string successScript;

    /// script to execute when complete_func() returns FAILURE
    std::string failureScript;

    /// script to execute when actions is completed
    std::string postScript;

    /// if async_delay > 0, this action become asynchronous and wait this amount of time
    std::chrono::milliseconds asyncDelay{std::chrono::milliseconds(0)};

    /// Function invoked when the action is completed. By default just return [return_status]
    /// Override it to intorduce more comple cases
    std::function<NodeStatus(void)> completeFunc = [this]() {
        return returnStatus;
    };
};

/**
 * @brief The TestNode is a Node that can be configure to:
 *
 * 1. Return a specific status (SUCCESS / FAILURE)
 * 2. Execute a post condition script (unless halted)
 * 3. Either complete immediately (synchronous action), or after a
 *    given period of time (asynchronous action)
 *
 * This behavior is changed by the parameters pased with TestNodeConfig.
 *
 * This particular node is created by the factory when TestNodeConfig is
 * added as a substitution rule:
 *
 *    TestNodeConfig test_config;
 *    // change fields of test_config
 *    factory.AddSubstitutionRule(pattern, test_config);
 *
 * See tutorial 11 for more details.
 */
class TestNode: public behaviortree::StatefulActionNode {
 public:
    TestNode(
            const std::string& refScript, const NodeConfig& refExecutor,
            TestNodeConfig testNodeConfig
    );

    static PortsList ProvidedPorts() {
        return {};
    }

 protected:
    virtual NodeStatus OnStart() override;

    virtual NodeStatus OnRunning() override;

    virtual void OnHalted() override;

    NodeStatus OnCompleted();

    TestNodeConfig m_TestConfig;
    ScriptFunction m_SuccessExecutor;
    ScriptFunction m_FailureExecutor;
    ScriptFunction m_PostExecutor;
    TimerQueue<> m_TimerQueue;
    std::atomic_bool m_Completed{false};
};

}// namespace behaviortree

#endif// BEHAVIORTREE_TEST_NODE_H
