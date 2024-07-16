#include "behaviortree/blackboard.h"

#include <unordered_set>

#include "behaviortree/json_export.h"

namespace behaviortree {

bool IsPrivateKey(std::string_view str) {
    return str.size() >= 1 && str.data()[0] == '_';
}

void Blackboard::EnableAutoRemapping(bool remapping) {
    m_AutoRemapping = remapping;
}

AnyPtrLocked Blackboard::GetAnyLocked(const std::string &refKey) {
    if(auto entry = GetEntry(refKey)) {
        return AnyPtrLocked(&entry->value, &entry->entryMutex);
    }
    return {};
}

AnyPtrLocked Blackboard::GetAnyLocked(const std::string &refKey) const {
    if(auto entry = GetEntry(refKey)) {
        return AnyPtrLocked(
                &entry->value, const_cast<std::mutex *>(&entry->entryMutex)
        );
    }
    return {};
}

const Any *Blackboard::GetAny(const std::string &refKey) const {
    return GetAnyLocked(refKey).Get();
}

Any *Blackboard::GetAny(const std::string &refKey) {
    return const_cast<Any *>(GetAnyLocked(refKey).Get());
}

const std::shared_ptr<Blackboard::Entry> Blackboard::GetEntry(const std::string &refKey) const {
    // special syntax: "@" will always refer to the root BB
    if(StartWith(refKey, '@')) {
        return GetRootBlackboard()->GetEntry(refKey.substr(1, refKey.size() - 1)
        );
    }

    std::unique_lock<std::mutex> lock(m_Mutex);
    auto ptrIt = m_Storage.find(refKey);
    if(ptrIt != m_Storage.end()) {
        return ptrIt->second;
    }
    // not found. Try autoremapping
    if(auto ptrParent = m_ParentBlackboard.lock()) {
        auto ptrRemapIt = m_InternalToExternal.find(refKey);
        if(ptrRemapIt != m_InternalToExternal.cend()) {
            auto const &refNewKey = ptrRemapIt->second;
            return ptrParent->GetEntry(refNewKey);
        }
        if(m_AutoRemapping && !IsPrivateKey(refKey)) {
            return ptrParent->GetEntry(refKey);
        }
    }
    return {};
}

std::shared_ptr<Blackboard::Entry> Blackboard::GetEntry(const std::string &refKey) {
    return static_cast<const Blackboard &>(*this).GetEntry(refKey);
}

const TypeInfo *Blackboard::GetEntryInfo(const std::string &refKey) {
    auto ptrEntry = GetEntry(refKey);
    return (ptrEntry == nullptr) ? nullptr : &(ptrEntry->typeInfo);
}

void Blackboard::AddSubtreeRemapping(std::string_view internal, std::string_view external) {
    m_InternalToExternal.insert({static_cast<std::string>(internal), static_cast<std::string>(external)});
}

void Blackboard::DebugMessage() const {
    for(const auto &[key, entry]: m_Storage) {
        auto portType = entry->typeInfo.Type();
        if(portType == typeid(void)) {
            portType = entry->value.Type();
        }

        std::cout << key << " (" << behaviortree::Demangle(portType) << ")"
                  << std::endl;
    }

    for(const auto &[from, to]: m_InternalToExternal) {
        std::cout << "[" << from << "] remapped to port of parent tree [" << to << "]" << std::endl;
        continue;
    }
}

std::vector<std::string_view> Blackboard::GetKeys() const {
    if(m_Storage.empty()) {
        return {};
    }
    std::vector<std::string_view> out;
    out.reserve(m_Storage.size());
    for(const auto &refEntryIt: m_Storage) {
        out.push_back(refEntryIt.first);
    }
    return out;
}

void Blackboard::Clear() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Storage.clear();
}

std::recursive_mutex &Blackboard::EntryMutex() const {
    return m_EntryMutex;
}

void Blackboard::CreateEntry(const std::string &refKey, const TypeInfo &refTypeInfo) {
    if(StartWith(refKey, '@')) {
        GetRootBlackboard()->CreateEntryImpl(refKey.substr(1, refKey.size() - 1), refTypeInfo);
    } else {
        CreateEntryImpl(refKey, refTypeInfo);
    }
}

void Blackboard::CloneInto(Blackboard &refDst) const {
    std::unique_lock lk1(m_Mutex);
    std::unique_lock lk2(refDst.m_Mutex);

    // keys that are not updated must be removed.
    std::unordered_set<std::string> keysToRemoveSet;
    auto &refDstStorage = refDst.m_Storage;
    for(const auto &[key, _]: refDstStorage) {
        keysToRemoveSet.insert(key);
    }

    // update or create entries in dst_storage
    for(const auto &[srcKey, srcEntry]: m_Storage) {
        keysToRemoveSet.erase(srcKey);

        auto ptrIt = refDstStorage.find(srcKey);
        if(ptrIt != refDstStorage.end()) {
            // overwite
            auto &refDstEntry = ptrIt->second;
            refDstEntry->stringConverter = srcEntry->stringConverter;
            refDstEntry->value = srcEntry->value;
            refDstEntry->typeInfo = srcEntry->typeInfo;
            refDstEntry->sequenceId++;
            refDstEntry->stamp = std::chrono::steady_clock::now().time_since_epoch();
        } else {
            // create new
            auto ptrNewEntry = std::make_shared<Entry>(srcEntry->typeInfo);
            ptrNewEntry->value = srcEntry->value;
            ptrNewEntry->stringConverter = srcEntry->stringConverter;
            refDstStorage.insert({srcKey, ptrNewEntry});
        }
    }

    for(const auto &refKey: keysToRemoveSet) {
        refDstStorage.erase(refKey);
    }
}

Blackboard::Ptr Blackboard::Parent() {
    if(auto ptrParent = m_ParentBlackboard.lock()) {
        return ptrParent;
    }
    return {};
}

std::shared_ptr<Blackboard::Entry> Blackboard::CreateEntryImpl(const std::string &refKey, const TypeInfo &refInfo) {
    std::unique_lock<std::mutex> lock(m_Mutex);
    // This function might be called recursively, when we do remapping, because we move
    // to the top scope to find already existing  entries

    // search if exists already
    auto storage_it = m_Storage.find(refKey);
    if(storage_it != m_Storage.end()) {
        const auto &refPreInfo = storage_it->second->typeInfo;
        if(refPreInfo.Type() != refInfo.Type() &&
           refPreInfo.IsStronglyTyped() && refInfo.IsStronglyTyped()) {
            auto msg = StrCat("Blackboard entry [", refKey,
                              "]: once declared, the Type of a port"
                              " shall not change. Previously declared Type [",
                              behaviortree::Demangle(refPreInfo.Type()), "], current Type [", behaviortree::Demangle(refInfo.Type()), "]");

            throw LogicError(msg);
        }
        return storage_it->second;
    }

    // manual remapping first
    auto ptrRemappingIt = m_InternalToExternal.find(refKey);
    if(ptrRemappingIt != m_InternalToExternal.end()) {
        const auto &refRemappedKey = ptrRemappingIt->second;
        if(auto ptrParent = m_ParentBlackboard.lock()) {
            return ptrParent->CreateEntryImpl(refRemappedKey, refInfo);
        }
        throw RuntimeError("Missing parent blackboard");
    }
    // autoremapping second (excluding private keys)
    if(m_AutoRemapping && !IsPrivateKey(refKey)) {
        if(auto ptrParent = m_ParentBlackboard.lock()) {
            return ptrParent->CreateEntryImpl(refKey, refInfo);
        }
        throw RuntimeError("Missing parent blackboard");
    }
    // not remapped, not found. Create locally.

    auto refEntry = std::make_shared<Entry>(refInfo);
    // even if empty, let's assign to it a default type
    refEntry->value = Any(refInfo.Type());
    m_Storage.insert({refKey, refEntry});
    return refEntry;
}

nlohmann::json ExportBlackboardToJSON(const Blackboard &blackboard) {
    nlohmann::json dest;
    for(auto entryName: blackboard.GetKeys()) {
        std::string name(entryName);
        if(auto anyRef = blackboard.GetAnyLocked(name)) {
            if(auto anyPtr = anyRef.Get()) {
                JsonExporter::Get().ToJson(*anyPtr, dest[name]);
            }
        }
    }
    return dest;
}

void ImportBlackboardFromJSON(const nlohmann::json &refJson, Blackboard &refBlackboard) {
    for(auto it = refJson.begin(); it != refJson.end(); ++it) {
        if(auto res = JsonExporter::Get().FromJson(it.value())) {
            auto ptrEntry = refBlackboard.GetEntry(it.key());
            if(ptrEntry == nullptr) {
                refBlackboard.CreateEntry(it.key(), res->second);
                ptrEntry = refBlackboard.GetEntry(it.key());
            }
            ptrEntry->value = res->first;
        }
    }
}

Blackboard::Entry &Blackboard::Entry::operator=(const Entry &refOther) {
    value = refOther.value;
    typeInfo = refOther.typeInfo;
    stringConverter = refOther.stringConverter;
    sequenceId = refOther.sequenceId;
    stamp = refOther.stamp;
    return *this;
}

Blackboard *behaviortree::Blackboard::GetRootBlackboard() {
    auto ptrBlackboard = static_cast<const Blackboard &>(*this).GetRootBlackboard();
    return const_cast<Blackboard *>(ptrBlackboard);
}

const Blackboard *behaviortree::Blackboard::GetRootBlackboard() const {
    const Blackboard *ptrBlackboard = this;
    Blackboard::Ptr ptrPreBlackboard = m_ParentBlackboard.lock();
    while(ptrPreBlackboard) {
        ptrBlackboard = ptrPreBlackboard.get();
        ptrPreBlackboard = ptrBlackboard->m_ParentBlackboard.lock();
    }
    return ptrBlackboard;
}

}// namespace behaviortree
