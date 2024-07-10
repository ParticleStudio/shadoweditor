#include "behaviortree/action/updated_action.h"

#include "behaviortree/factory.h"

namespace behaviortree {
EntryUpdatedAction::EntryUpdatedAction(const std::string& refName, const NodeConfig& refConfig)
    : SyncActionNode(refName, refConfig) {
    auto it = refConfig.inputPortsMap.find("entry");
    if(it == refConfig.inputPortsMap.end() || it->second.empty()) {
        throw LogicError("Missing port 'entry' in ", refName);
    }
    const auto entryStr = it->second;
    StringView strippedKey;
    if(IsBlackboardPointer(entryStr, &strippedKey)) {
        m_EntryKey = strippedKey;
    } else {
        m_EntryKey = entryStr;
    }
}

NodeStatus EntryUpdatedAction::Tick() {
    if(auto ptrEntry = GetConfig().ptrBlackboard->GetEntry(m_EntryKey)) {
        std::unique_lock lk(ptrEntry->entryMutex);
        const uint64_t currentId = ptrEntry->sequenceId;
        const uint64_t previousId = m_SequenceId;
        m_SequenceId = currentId;
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
        return (previousId != currentId) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    } else {
        return NodeStatus::FAILURE;
    }
}
}// namespace behaviortree
