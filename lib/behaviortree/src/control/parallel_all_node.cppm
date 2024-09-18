export module behaviortree.parallel_all_node;

import <set>;

import behaviortree.control_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The ParallelAllNode execute all its GetChildrenNode
 * __concurrently__, but not in separate threads!
 *
 * It differs in the way ParallelNode works because the latter may stop
 * and halt other GetChildrenNode if a certain number of SUCCESS/FAILURES is reached,
 * whilst this one will always complete the execution of ALL its GetChildrenNode.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of GetChildrenNode.
 */
export class BEHAVIORTREE_API ParallelAllNode: public ControlNode {
 public:
    ParallelAllNode(const std::string &rName, const NodeConfig &rConfig);

    static PortMap ProvidedPorts() {
        return {InputPort<int>("max_failures", 1,"If the number of GetChildrenNode returning FAILURE exceeds this value, ParallelAll returns FAILURE")};
    }

    ~ParallelAllNode() override = default;

    virtual void Halt() override;

    size_t GetFailureThreshold() const;
    void SetFailureThreshold(int32_t threshold);

 private:
    size_t m_failureThreshold;

    std::set<size_t> m_completedSet;
    size_t m_failureCount{0};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace behaviortree

// module behaviortree.parallel_all_node;
