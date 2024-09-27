#include "behaviortree/action/updated_action.hpp"

import common.exception;

#include "behaviortree/factory.h"

namespace behaviortree {
EntryUpdatedAction::EntryUpdatedAction(const std::string &rName, const NodeConfig &rConfig): SyncActionNode(rName, rConfig) {
    auto it = rConfig.inputPortMap.find("entry");
    if(it == rConfig.inputPortMap.end() || it->second.empty()) {
        throw util::LogicError("Missing port 'entry' in ", rName);
    }
    const auto entryStr = it->second;
    std::string_view strippedKey;
    if(IsBlackboardPointer(entryStr, &strippedKey)) {
        m_entryKey = strippedKey;
    } else {
        m_entryKey = entryStr;
    }
}

NodeStatus EntryUpdatedAction::Tick() {
    if(auto pEntry = GetConfig().pBlackboard->GetEntry(m_entryKey)) {
        std::unique_lock lk(pEntry->entryMutex);
        const uint64_t curSequenceId = pEntry->sequenceId;
        const uint64_t preSequenceId = m_sequenceId;
        m_sequenceId = curSequenceId;
        /*
    uint64_t previous_id = 0;
    auto& previous_id_registry = details::GlobalSequenceRegistry();

    // find the previous id in the registry.
    auto it = previous_id_registry.find(entry.get());
    if(it != previous_id_registry.end())
    {
      previous_id = it->second;
    }
    if(previous_id != current_id)
    {
      previous_id_registry[entry.get()] = current_id;
    }*/
        return (preSequenceId != curSequenceId) ? NodeStatus::Success
                                                : NodeStatus::Failure;
    } else {
        return NodeStatus::Failure;
    }
}
}// namespace behaviortree
