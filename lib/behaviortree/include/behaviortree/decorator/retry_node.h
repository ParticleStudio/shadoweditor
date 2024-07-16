#ifndef BEHAVIORTREE_RETRY_NODE_H
#define BEHAVIORTREE_RETRY_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The RetryNode is used to execute a child several times if it fails.
 *
 * If the child returns SUCCESS, the loop is stopped and this node
 * returns SUCCESS.
 *
 * If the child returns FAILURE, this node will try again up to N times
 * (N is read from port "num_attempts").
 *
 * Example:
 *
 * <RetryUntilSuccessful num_attempts="3">
 *     <OpenDoor/>
 * </RetryUntilSuccessful>
 *
 * Note:
 * RetryNodeTypo is only included to support the deprecated typo
 * "RetryUntilSuccesful" (note the single 's' in Succesful)
 */
class RetryNode: public DecoratorNode {
 public:
    RetryNode(const std::string &refName, int NTries);

    RetryNode(const std::string &refName, const NodeConfig &refConfig);

    virtual ~RetryNode() override = default;

    static PortMap ProvidedPorts() {
        return {InputPort<int>(
                NUM_ATTEMPTS,
                "Execute again a failing child up to N times. "
                "Use -1 to create an infinite loop."
        )};
    }

    virtual void Halt() override;

 private:
    int m_MaxAttempts;
    int m_TryCount;
    bool m_AllSkipped{true};

    bool m_ReadParameterFromPorts;
    static constexpr const char *NUM_ATTEMPTS{"num_attempts"};

    virtual behaviortree::NodeStatus Tick() override;
};

class [[deprecated(
        "RetryUntilSuccesful was a typo and deprecated, use "
        "RetryUntilSuccessful "
        "instead."
)]] RetryNodeTypo: public RetryNode {
 public:
    RetryNodeTypo(const std::string &refName, int NTries): RetryNode(refName, NTries){};

    RetryNodeTypo(const std::string &refName, const NodeConfig &refConfig): RetryNode(refName, refConfig){};

    virtual ~RetryNodeTypo() override = default;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_RETRY_NODE_H
