#include "behaviortree/blackboard.h"

#include <unordered_set>

#include "behaviortree/json_export.h"

namespace behaviortree {

bool IsPrivateKey(std::string_view str) {
    return str.size() >= 1 and str.data()[0] == '_';
}

void Blackboard::EnableAutoRemapping(bool remapping) {
    m_autoRemapping = remapping;
}

AnyPtrLocked Blackboard::GetAnyLocked(const std::string &rKey) {
    if(auto pEntry = GetEntry(rKey)) {
        return AnyPtrLocked(&pEntry->value, &pEntry->entryMutex);
    }
    return {};
}

AnyPtrLocked Blackboard::GetAnyLocked(const std::string &rKey) const {
    if(auto pEntry = GetEntry(rKey)) {
        return AnyPtrLocked(&pEntry->value, const_cast<std::mutex *>(&pEntry->entryMutex));
    }
    return {};
}

const std::shared_ptr<Blackboard::Entry> Blackboard::GetEntry(const std::string &rKey) const {
    // special syntax: "@" will always refer to the root BB
    if(StartWith(rKey, '@')) {
        return GetRootBlackboard()->GetEntry(rKey.substr(1, rKey.size() - 1));
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    auto iter = m_storageMap.find(rKey);
    if(iter != m_storageMap.end()) {
        return iter->second;
    }
    // not found. Try autoremapping
    if(auto pParent = m_pParentBlackboard.lock()) {
        auto pRemapIt = m_internalToExternalMap.find(rKey);
        if(pRemapIt != m_internalToExternalMap.cend()) {
            auto const &rNewKey = pRemapIt->second;
            return pParent->GetEntry(rNewKey);
        }
        if(m_autoRemapping and !IsPrivateKey(rKey)) {
            return pParent->GetEntry(rKey);
        }
    }
    return {};
}

std::shared_ptr<Blackboard::Entry> Blackboard::GetEntry(const std::string &rKey) {
    return static_cast<const Blackboard &>(*this).GetEntry(rKey);
}

const TypeInfo *Blackboard::GetEntryInfo(const std::string &rKey) {
    auto pEntry = GetEntry(rKey);
    return (pEntry == nullptr) ? nullptr : &(pEntry->typeInfo);
}

void Blackboard::AddSubtreeRemapping(std::string_view internal, std::string_view external) {
    m_internalToExternalMap.insert({static_cast<std::string>(internal), static_cast<std::string>(external)});
}

void Blackboard::DebugMessage() const {
    for(const auto &[key, pEntry]: m_storageMap) {
        auto portType = pEntry->typeInfo.Type();
        if(portType == typeid(void)) {
            portType = pEntry->value.Type();
        }

        std::cout << key << " (" << behaviortree::Demangle(portType) << ")" << std::endl;
    }

    for(const auto &[from, to]: m_internalToExternalMap) {
        std::cout << "[" << from << "] remapped to port of parent tree [" << to << "]" << std::endl;
        continue;
    }
}

std::vector<std::string_view> Blackboard::GetKeys() const {
    if(m_storageMap.empty()) {
        return {};
    }
    std::vector<std::string_view> out;
    out.reserve(m_storageMap.size());
    for(const auto &refEntryIt: m_storageMap) {
        out.push_back(refEntryIt.first);
    }
    return out;
}

void Blackboard::CreateEntry(const std::string &rKey, const TypeInfo &rTypeInfo) {
    if(StartWith(rKey, '@')) {
        GetRootBlackboard()->CreateEntryImpl(rKey.substr(1, rKey.size() - 1), rTypeInfo);
    } else {
        CreateEntryImpl(rKey, rTypeInfo);
    }
}

void Blackboard::CloneInto(Blackboard &rDst) const {
    std::unique_lock lock1(m_mutex);
    std::unique_lock lock2(rDst.m_mutex);

    // keys that are not updated must be removed.
    std::unordered_set<std::string> keysToRemoveSet;
    auto &rDstStorage = rDst.m_storageMap;
    for(const auto &[key, _]: rDstStorage) {
        keysToRemoveSet.insert(key);
    }

    // update or create entries in dst_storage
    for(const auto &[srcKey, pSrcEntry]: m_storageMap) {
        keysToRemoveSet.erase(srcKey);

        auto pIt = rDstStorage.find(srcKey);
        if(pIt != rDstStorage.end()) {
            // overwite
            auto &rDstEntry = pIt->second;
            rDstEntry->stringConverter = pSrcEntry->stringConverter;
            rDstEntry->value = pSrcEntry->value;
            rDstEntry->typeInfo = pSrcEntry->typeInfo;
            rDstEntry->sequenceId++;
            rDstEntry->stamp = std::chrono::steady_clock::now().time_since_epoch();
        } else {
            // create new
            auto pNewEntry = std::make_shared<Entry>(pSrcEntry->typeInfo);
            pNewEntry->value = pSrcEntry->value;
            pNewEntry->stringConverter = pSrcEntry->stringConverter;
            rDstStorage.insert({srcKey, pNewEntry});
        }
    }

    for(const auto &rKey: keysToRemoveSet) {
        rDstStorage.erase(rKey);
    }
}

Blackboard::Ptr Blackboard::Parent() {
    if(auto pParent = m_pParentBlackboard.lock()) {
        return pParent;
    }
    return {};
}

std::shared_ptr<Blackboard::Entry> Blackboard::CreateEntryImpl(const std::string &rKey, const TypeInfo &rInfo) {
    std::unique_lock<std::mutex> lock(m_mutex);
    // This function might be called recursively, when we do remapping, because we move
    // to the top scope to find already existing  entries

    // search if exists already
    auto storageIter = m_storageMap.find(rKey);
    if(storageIter != m_storageMap.end()) {
        const auto &rPreInfo = storageIter->second->typeInfo;
        if(rPreInfo.Type() != rInfo.Type() and
           rPreInfo.IsStronglyTyped() and rInfo.IsStronglyTyped()) {
            auto msg = util::StrCat("Blackboard entry [", rKey,
                              "]: once declared, the Type of a port"
                              " shall not change. Previously declared Type [",
                              behaviortree::Demangle(rPreInfo.Type()), "], current Type [", behaviortree::Demangle(rInfo.Type()), "]");

            throw util::LogicError(msg);
        }
        return storageIter->second;
    }

    // manual remapping first
    auto ptrRemappingIter = m_internalToExternalMap.find(rKey);
    if(ptrRemappingIter != m_internalToExternalMap.end()) {
        const auto &rRemappedKey = ptrRemappingIter->second;
        if(auto pParent = m_pParentBlackboard.lock()) {
            return pParent->CreateEntryImpl(rRemappedKey, rInfo);
        }
        throw util::RuntimeError("Missing parent blackboard");
    }
    // autoremapping second (excluding private keys)
    if(m_autoRemapping and !IsPrivateKey(rKey)) {
        if(auto pParent = m_pParentBlackboard.lock()) {
            return pParent->CreateEntryImpl(rKey, rInfo);
        }
        throw util::RuntimeError("Missing parent blackboard");
    }
    // not remapped, not found. Create locally.

    auto pEntry = std::make_shared<Entry>(rInfo);
    // even if empty, let's assign to it a default type
    pEntry->value = Any(rInfo.Type());
    m_storageMap.insert({rKey, pEntry});
    return pEntry;
}

nlohmann::json ExportBlackboardToJson(const Blackboard &rBlackboard) {
    nlohmann::json dest;
    for(auto entryName: rBlackboard.GetKeys()) {
        std::string name(entryName);
        if(auto anyRef = rBlackboard.GetAnyLocked(name)) {
            if(auto pAny = anyRef.Get()) {
                JsonExporter::Get().ToJson(*pAny, dest[name]);
            }
        }
    }
    return dest;
}

void ImportBlackboardFromJson(const nlohmann::json &rJson, Blackboard &rBlackboard) {
    for(auto iter = rJson.begin(); iter != rJson.end(); ++iter) {
        if(auto res = JsonExporter::Get().FromJson(iter.value())) {
            auto pEntry = rBlackboard.GetEntry(iter.key());
            if(pEntry == nullptr) {
                rBlackboard.CreateEntry(iter.key(), res->second);
                pEntry = rBlackboard.GetEntry(iter.key());
            }
            pEntry->value = res->first;
        }
    }
}

Blackboard::Entry &Blackboard::Entry::operator=(const Entry &rOther) {
    value = rOther.value;
    typeInfo = rOther.typeInfo;
    stringConverter = rOther.stringConverter;
    sequenceId = rOther.sequenceId;
    stamp = rOther.stamp;
    return *this;
}

Blackboard *behaviortree::Blackboard::GetRootBlackboard() {
    auto pBlackboard = static_cast<const Blackboard &>(*this).GetRootBlackboard();
    return const_cast<Blackboard *>(pBlackboard);
}

const Blackboard *behaviortree::Blackboard::GetRootBlackboard() const {
    const Blackboard *pBlackboard = this;
    Blackboard::Ptr pPreBlackboard = m_pParentBlackboard.lock();
    while(pPreBlackboard) {
        pBlackboard = pPreBlackboard.get();
        pPreBlackboard = pBlackboard->m_pParentBlackboard.lock();
    }
    return pBlackboard;
}

}// namespace behaviortree
