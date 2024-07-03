#ifndef BEHAVIORTREE_PARALLEL_NODE_H
#define BEHAVIORTREE_PARALLEL_NODE_H

#include <set>

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ParallelNode execute all its children
 * __concurrently__, but not in separate threads!
 *
 * Even if this may look similar to ReactiveSequence,
 * this Control Node is the __only__ one that can have
 * multiple children RUNNING at the same time.
 *
 * The Node is completed either when the THRESHOLD_SUCCESS
 * or THRESHOLD_FAILURE number is reached (both configured using ports).
 *
 * If any of the thresholds is reached, and other children are still running,
 * they will be halted.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of children.
 */
class ParallelNode: public ControlNode {
 public:
    ParallelNode(const std::string& refName);

    ParallelNode(const std::string& refName, const NodeConfig& refConfig);

    static PortsList ProvidedPorts() {
        return {InputPort<int>(THRESHOLD_SUCCESS, -1,
                               "number of children that need to succeed to trigger a "
                               "SUCCESS"),
                InputPort<int>(THRESHOLD_FAILURE, 1,
                               "number of children that need to fail to trigger a "
                               "FAILURE")};
    }

    ~ParallelNode() override = default;

    virtual void Halt() override;

    size_t SuccessThreshold() const;
    size_t failureThreshold() const;
    void SetSuccessThreshold(int threshold);
    void SetFailureThreshold(int threshold);

 private:
    int m_SuccessThreshold;
    int m_FailureThreshold;

    std::set<size_t> m_CompletedList;

    size_t m_SuccessCount{0};
    size_t m_FailureCount{0};

    bool m_ReadParameterFromPorts;
    static constexpr const char* THRESHOLD_SUCCESS{"success_count"};
    static constexpr const char* THRESHOLD_FAILURE{"failure_count"};

    virtual behaviortree::NodeStatus Tick() override;

    void Clear();
};

}// namespace behaviortree

#endif// BEHAVIORTREE_PARALLEL_NODE_H
