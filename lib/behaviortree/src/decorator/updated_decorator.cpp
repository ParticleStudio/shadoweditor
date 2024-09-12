#include "behaviortree/decorator/updated_decorator.h"

#include "behaviortree/factory.h"

namespace behaviortree {
EntryUpdatedDecorator::EntryUpdatedDecorator(const std::string &rName, const NodeConfig &rConfig, NodeStatus ifNotUpdated): DecoratorNode(rName, rConfig),
                                                                                                                                m_ifNotUpdated(ifNotUpdated) {
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

NodeStatus EntryUpdatedDecorator::Tick() {
    // continue executing an asynchronous child
    if(m_stillExecutingChild) {
        auto status = GetChildNode()->ExecuteTick();
        m_stillExecutingChild = (status == NodeStatus::Running);
        return status;
    }

    if(auto entry = GetConfig().pBlackboard->GetEntry(m_entryKey)) {
        std::unique_lock lock(entry->entryMutex);
        const uint64_t curSequenceId = entry->sequenceId;
        const uint64_t preSequenceId = m_sequenceId;
        m_sequenceId = curSequenceId;

        if(preSequenceId == curSequenceId) {
            return m_ifNotUpdated;
        }
    } else {
        return m_ifNotUpdated;
    }

    auto status = GetChildNode()->ExecuteTick();
    m_stillExecutingChild = (status == NodeStatus::Running);
    return status;
}

void EntryUpdatedDecorator::Halt() {
    m_stillExecutingChild = false;
}
}// namespace behaviortree
