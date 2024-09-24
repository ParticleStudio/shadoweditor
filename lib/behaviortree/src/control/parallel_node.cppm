module;

export module behaviortree.parallel_node;

import <set>;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The ParallelNode execute all its GetChildrenNode
 * __concurrently__, but not in separate threads!
 *
 * Even if this may look similar to ReactiveSequence,
 * this Control Node is the __only__ one that can have
 * multiple GetChildrenNode RUNNING at the same time.
 *
 * The Node is completed either when the THRESHOLD_SUCCESS
 * or THRESHOLD_FAILURE number is reached (both configured using ports).
 *
 * If any of the thresholds is reached, and other GetChildrenNode are still running,
 * they will be halted.
 *
 * Note that threshold indexes work as in Python:
 * https://www.i2tutorials.com/what-are-negative-indexes-and-why-are-they-used/
 *
 * Therefore -1 is equivalent to the number of GetChildrenNode.
 */
export class BEHAVIORTREE_API ParallelNode: public ControlNode {
 public:
    ParallelNode(const std::string &rName);

    ParallelNode(const std::string &rName, const NodeConfig &rConfig);

    static PortMap ProvidedPorts() {
        return {
                InputPort<int32_t>(THRESHOLD_SUCCESS, -1, "number of GetChildrenNode that need to succeed to trigger a Success"),
                InputPort<int32_t>(THRESHOLD_FAILURE, 1, "number of GetChildrenNode that need to fail to trigger a Failure")
        };
    }

    ~ParallelNode() override = default;

    virtual void Halt() override;

    size_t SuccessThreshold() const;
    size_t FailureThreshold() const;
    void SetSuccessThreshold(int32_t threshold);
    void SetFailureThreshold(int32_t threshold);

 private:
    int m_successThreshold;
    int m_failureThreshold;

    std::set<size_t> m_completedSet;

    size_t m_successCount{0};
    size_t m_failureCount{0};

    bool m_readParameterFromPorts;
    static constexpr const char *THRESHOLD_SUCCESS{"success_count"};
    static constexpr const char *THRESHOLD_FAILURE{"failure_count"};

    virtual behaviortree::NodeStatus Tick() override;

    void Clear();
};

}// namespace behaviortree

// module behaviortree.parallel_node;
// module;
