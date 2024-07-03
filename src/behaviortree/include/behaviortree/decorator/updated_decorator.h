#ifndef BEHAVIORTREE_UPDATED_DECORATOR_H
#define BEHAVIORTREE_UPDATED_DECORATOR_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The EntryUpdatedDecorator checks the Timestamp in an entry
 * to determine if the value was updated since the last time (true,
 * the first time).
 *
 * If it is, the child will be executed, otherwise [if_not_updated] value is returned.
 */
class EntryUpdatedDecorator: public DecoratorNode {
 public:
    EntryUpdatedDecorator(const std::string& refName, const NodeConfig& refConfig,
                          NodeStatus ifNotUpdated);

    ~EntryUpdatedDecorator() override = default;

    static PortsList ProvidedPorts() {
        return {InputPort<behaviortree::Any>("entry", "Entry to check")};
    }

 private:
    uint64_t m_SequenceId{0};
    std::string m_EntryKey;
    bool m_StillExecutingChild{false};
    NodeStatus m_IfNotUpdated;

    NodeStatus Tick() override;

    void Halt() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_UPDATED_DECORATOR_H
