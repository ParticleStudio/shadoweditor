export module behaviortree.repeat_node;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The RepeatNode is used to execute a GetChildNode several times, as long
 * as it succeed.
 *
 * To succeed, the GetChildNode must return SUCCESS N times (port "num_cycles").
 *
 * If the GetChildNode returns FAILURE, the loop is stopped and this node
 * returns FAILURE.
 *
 * Example:
 *
 * <Repeat num_cycles="3">
 *   <ClapYourHandsOnce/>
 * </Repeat>
 */
export class BEHAVIORTREE_API RepeatNode: public DecoratorNode {
 public:
    RepeatNode(const std::string &rName, int NTries);

    RepeatNode(const std::string &rName, const NodeConfig &rConfig);

    virtual ~RepeatNode() override = default;

    static PortMap ProvidedPorts() {
        return {InputPort<int>(NUM_CYCLES, "Repeat a successful GetChildNode up to N times. Use -1 to create an infinite loop.")};
    }

 private:
    int m_numCycles;
    int m_repeatCount;
    bool m_allSkipped{true};

    bool m_readParameterFromPorts;
    static constexpr const char *NUM_CYCLES{"num_cycles"};

    virtual NodeStatus Tick() override;

    void Halt() override;
};

}// namespace behaviortree

// module behaviortree.repeat_node;
