#ifndef BEHAVIORTREE_PARALLEL_ALL_NODE_H
#define BEHAVIORTREE_PARALLEL_ALL_NODE_H

#include <set>

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ParallelAllNode execute all its children
 * __concurrently__, but not in separate threads!
 *
 * It differs in the way ParallelNode works because the latter may stop
 * and halt other children if a certain number of SUCCESS/FAILURES is reached,
 * whilst this one will always complete the execution of ALL its children.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of children.
 */
class ParallelAllNode: public ControlNode {
 public:
    ParallelAllNode(const std::string& refName, const NodeConfig& refConfig);

    static PortsList ProvidedPorts() {
        return {InputPort<int>("max_failures", 1,
                               "If the number of children returning FAILURE exceeds this "
                               "value, "
                               "ParallelAll returns FAILURE")};
    }

    ~ParallelAllNode() override = default;

    virtual void Halt() override;

    size_t FailureThreshold() const;
    void SetFailureThreshold(int threshold);

 private:
    size_t m_FailureThreshold;

    std::set<size_t> m_CompletedList;
    size_t m_FailureCount{0};

    virtual behaviortree::NodeStatus Tick() override;
};

}// namespace BT

#endif// BEHAVIORTREE_PARALLEL_ALL_NODE_H
