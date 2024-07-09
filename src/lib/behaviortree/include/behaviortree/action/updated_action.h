#ifndef BEHAVIORTREE_UPDATED_ACTION_H
#define BEHAVIORTREE_UPDATED_ACTION_H

#include "behaviortree/action_node.h"

namespace behaviortree {
/**
 * @brief The EntryUpdatedAction checks the Timestamp in an entry
 * to determine if the value was updated since the last time.
 *
 * SUCCESS if it was updated, since the last time it was checked,
 * FAILURE if it doesn't exist or was not updated.
 */
class EntryUpdatedAction: public SyncActionNode {
 public:
    EntryUpdatedAction(const std::string& refName, const NodeConfig& refConfig);

    ~EntryUpdatedAction() override = default;

    static PortsList ProvidedPorts() {
        return {InputPort<behaviortree::Any>("entry", "Entry to check")};
    }

 private:
    uint64_t m_SequenceId = 0;
    std::string m_EntryKey;

    NodeStatus Tick() override;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_UPDATED_ACTION_H
