#ifndef BEHAVIORTREE_UPDATED_DECORATOR_H
#define BEHAVIORTREE_UPDATED_DECORATOR_H

#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * @brief The EntryUpdatedDecorator checks the Timestamp in an entry
 * to determine if the value was updated since the last time (true,
 * the first time).
 *
 * If it is, the GetChildNode will be executed, otherwise [if_not_updated] value is returned.
 */
class EntryUpdatedDecorator: public DecoratorNode {
 public:
    EntryUpdatedDecorator(const std::string &rName, const NodeConfig &rConfig, NodeStatus ifNotUpdated);

    ~EntryUpdatedDecorator() override = default;

    static PortMap ProvidedPorts() {
        return {InputPort<behaviortree::Any>("entry", "Entry to check")};
    }

 private:
    uint64_t m_sequenceId{0};
    std::string m_entryKey;
    bool m_stillExecutingChild{false};
    NodeStatus m_ifNotUpdated;

    NodeStatus Tick() override;

    void Halt() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_UPDATED_DECORATOR_H
