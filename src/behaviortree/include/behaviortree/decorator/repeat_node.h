#ifndef BEHAVIORTREE_REPEAT_NODE_H
#define BEHAVIORTREE_REPEAT_NODE_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The RepeatNode is used to execute a Child several times, as long
 * as it succeed.
 *
 * To succeed, the Child must return SUCCESS N times (port "num_cycles").
 *
 * If the Child returns FAILURE, the loop is stopped and this node
 * returns FAILURE.
 *
 * Example:
 *
 * <Repeat num_cycles="3">
 *   <ClapYourHandsOnce/>
 * </Repeat>
 */
class RepeatNode: public DecoratorNode {
 public:
    RepeatNode(const std::string& refName, int NTries);

    RepeatNode(const std::string& refName, const NodeConfig& refConfig);

    virtual ~RepeatNode() override = default;

    static PortsList ProvidedPorts() {
        return {InputPort<int>(NUM_CYCLES,
                               "Repeat a successful Child up to N times. "
                               "Use -1 to create an infinite loop.")};
    }

 private:
    int m_NumCycles;
    int m_RepeatCount;
    bool m_AllSkipped{true};

    bool m_ReadParameterFromPorts;
    static constexpr const char* NUM_CYCLES{"num_cycles"};

    virtual NodeStatus Tick() override;

    void Halt() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_REPEAT_NODE_H
