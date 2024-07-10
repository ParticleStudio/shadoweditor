#include "behaviortree/decorator/updated_decorator.h"

#include "behaviortree/factory.h"

namespace behaviortree {
EntryUpdatedDecorator::EntryUpdatedDecorator(const std::string& refName, const NodeConfig& refConfig, NodeStatus ifNotUpdated)
    : DecoratorNode(refName, refConfig), m_IfNotUpdated(ifNotUpdated) {
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

NodeStatus EntryUpdatedDecorator::Tick() {
    // continue executing an asynchronous child
    if(m_StillExecutingChild) {
        auto status = GetChild()->ExecuteTick();
        m_StillExecutingChild = (status == NodeStatus::RUNNING);
        return status;
    }

    if(auto entry = GetConfig().ptrBlackboard->GetEntry(m_EntryKey)) {
        std::unique_lock lk(entry->entryMutex);
        const uint64_t currentId = entry->sequenceId;
        const uint64_t previousId = m_SequenceId;
        m_SequenceId = currentId;

        if(previousId == currentId) {
            return m_IfNotUpdated;
        }
    } else {
        return m_IfNotUpdated;
    }

    auto status = GetChild()->ExecuteTick();
    m_StillExecutingChild = (status == NodeStatus::RUNNING);
    return status;
}

void EntryUpdatedDecorator::Halt() {
    m_StillExecutingChild = false;
}
}// namespace behaviortree
