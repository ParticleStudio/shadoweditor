#ifndef BEHAVIORTREE_BLACKBOARD_H
#define BEHAVIORTREE_BLACKBOARD_H

import <memory>;
import <mutex>;
import <string>;
import <unordered_map>;

import common.exception;

#include "behaviortree/basic_types.h"
#include "behaviortree/behaviortree_common.h"
#include "behaviortree/util/locked_reference.hpp"
#include "behaviortree/util/safe_any.hpp"
#include "nlohmann/json.hpp"

namespace behaviortree {

/// This Type contains a pointer to Any, protected
/// with a locked mutex as long as the object is in scope
using AnyPtrLocked = LockedPtr<Any>;

template<typename T>
struct StampedValue {
    T value;
    Timestamp stamp;
};

/**
 * @brief The Blackboard is the mechanism used by BehaviorTrees to exchange
 * typed data.
 */
class Blackboard {
 public:
    using Ptr = std::shared_ptr<Blackboard>;

 protected:
    // This is intentionally protected. Use Blackboard::create instead
    Blackboard(Blackboard::Ptr pParentBlackboard): m_pParentBlackboard(pParentBlackboard) {}

 public:
    struct Entry {
        Any value;
        TypeInfo typeInfo;
        StringConverter stringConverter;
        mutable std::mutex entryMutex;

        uint64_t sequenceId{0};
        // timestamp since epoch
        std::chrono::nanoseconds stamp{std::chrono::nanoseconds{0}};

        Entry(const TypeInfo &rTypeInfo): typeInfo(rTypeInfo) {}

        Entry &operator=(const Entry &rOther);
    };

    /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
    static Blackboard::Ptr Create(Blackboard::Ptr pParentBlackboard = {}) {
        return std::shared_ptr<Blackboard>(new Blackboard(pParentBlackboard));
    }

    virtual ~Blackboard() = default;

    void EnableAutoRemapping(bool remapping);

    [[nodiscard]] const std::shared_ptr<Entry> GetEntry(const std::string &rKey) const;

    [[nodiscard]] std::shared_ptr<Blackboard::Entry> GetEntry(const std::string &rKey);

    [[nodiscard]] AnyPtrLocked GetAnyLocked(const std::string &rKey);

    [[nodiscard]] AnyPtrLocked GetAnyLocked(const std::string &rKey) const;

    /** Return true if the entry with the given key was found.
   *  Note that this method may throw an exception if the Cast to T failed.
   */
    template<typename T>
    [[nodiscard]] bool Get(const std::string &rKey, T &rValue) const;

    template<typename T>
    [[nodiscard]] Expected<Timestamp> GetStamped(const std::string &rKey, T &rValue) const;

    /**
   * Version of Get() that throws if it fails.
   */
    template<typename T>
    [[nodiscard]] T Get(const std::string &rKey) const;

    template<typename T>
    [[nodiscard]] Expected<StampedValue<T>> GetStamped(const std::string &rKey) const;

    /// Update the entry with the given key
    template<typename T>
    void Set(const std::string &rKey, const T &rValue);

    void Unset(const std::string &rKey);

    [[nodiscard]] const TypeInfo *GetEntryInfo(const std::string &rKey);

    void AddSubtreeRemapping(std::string_view internal, std::string_view external);

    void DebugMessage() const;

    [[nodiscard]] std::vector<std::string_view> GetKeys() const;

    void CreateEntry(const std::string &rKey, const TypeInfo &rTypeInfo);

    /**
   * @brief cloneInto copies the values of the entries
   * into another blackboard.  Known limitations:
   *
   * - it doesn't update the remapping in dst
   * - it doesn't change the parent blackboard os dst
   *
   * @param rDst destination, i.e. blackboard to be updated
   */
    void CloneInto(Blackboard &rDst) const;

    Blackboard::Ptr Parent();

    // recursively look for parent Blackboard, until you find the root
    Blackboard *GetRootBlackboard();

    const Blackboard *GetRootBlackboard() const;

 private:
    mutable std::mutex m_mutex;
    mutable std::recursive_mutex m_entryMutex;
    std::unordered_map<std::string, std::shared_ptr<Entry>> m_storageMap;
    std::weak_ptr<Blackboard> m_pParentBlackboard;
    std::unordered_map<std::string, std::string> m_internalToExternalMap;

    std::shared_ptr<Entry> CreateEntryImpl(const std::string &rKey, const TypeInfo &rInfo);

    bool m_autoRemapping{false};
};

/**
 * @brief ExportBlackboardToJson will create a Json
 * that contains the current values of the blackboard.
 * Complex types must be registered with JsonExporter::Get()
 */
nlohmann::json ExportBlackboardToJson(const Blackboard &rBlackboard);

/**
 * @brief ImportBlackboardFromJson will append elements to the blackboard,
 * using the values parsed from the Json file created using ExportBlackboardToJson.
 * Complex types must be registered with JsonExporter::Get()
 */
void ImportBlackboardFromJson(const nlohmann::json &rJson, Blackboard &rBlackboard);

//------------------------------------------------------

template<typename T>
inline T Blackboard::Get(const std::string &rKey) const {
    if(auto anyLocked = GetAnyLocked(rKey)) {
        const auto &rAny = anyLocked.Get();
        if(rAny->Empty()) {
            throw util::RuntimeError("Blackboard::Get() error. Entry [", rKey, "] hasn't been initialized, yet");
        }
        return anyLocked.Get()->Cast<T>();
    }
    throw util::RuntimeError("Blackboard::Get() error. Missing key [", rKey, "]");
}

inline void Blackboard::Unset(const std::string &rKey) {
    std::unique_lock lock(m_mutex);

    // check local storage
    auto it = m_storageMap.find(rKey);
    if(it == m_storageMap.end()) {
        // No entry, nothing to do.
        return;
    }

    m_storageMap.erase(it);
}

template<typename T>
inline void Blackboard::Set(const std::string &rKey, const T &rValue) {
    if(StartWith(rKey, '@')) {
        GetRootBlackboard()->Set(rKey.substr(1, rKey.size() - 1), rValue);
        return;
    }
    std::unique_lock lock(m_mutex);

    // check local storage
    auto it = m_storageMap.find(rKey);
    Any newValue(rValue);
    if(it == m_storageMap.end()) {
        lock.unlock();
        std::shared_ptr<Blackboard::Entry> entry;
        // if a new generic port is created with a string, it's type should be AnyTypeAllowed
        if constexpr(std::is_same_v<std::string, T>) {
            entry = CreateEntryImpl(rKey, PortInfo(PortDirection::InOut));
        } else {
            PortInfo newPort(
                    PortDirection::InOut, newValue.Type(),
                    GetAnyFromStringFunctor<T>()
            );
            entry = CreateEntryImpl(rKey, newPort);
        }
        lock.lock();

        entry->value = newValue;
        entry->sequenceId++;
        entry->stamp = std::chrono::steady_clock::now().time_since_epoch();
    } else {
        // this is not the first time we set this entry, we need to check
        // if the type is the same or not.
        Entry &rEntry = *it->second;
        std::scoped_lock scopedLock(rEntry.entryMutex);

        Any &rPreviousAny = rEntry.value;
        // special case: entry exists but it is not strongly typed... yet
        if(!rEntry.typeInfo.IsStronglyTyped()) {
            // Use the new type to create a new entry that is strongly typed.
            rEntry.typeInfo = TypeInfo::Create<T>();
            rEntry.sequenceId++;
            rEntry.stamp = std::chrono::steady_clock::now().time_since_epoch();
            rPreviousAny = std::move(newValue);
            return;
        }

        std::type_index previousType = rEntry.typeInfo.Type();

        // check type mismatch
        if(previousType != std::type_index(typeid(T)) &&
           previousType != newValue.Type()) {
            bool mismatching = true;
            if(std::is_constructible<std::string_view, T>::value) {
                Any anyFromString = rEntry.typeInfo.ParseString(rValue);
                if(anyFromString.Empty() == false) {
                    mismatching = false;
                    newValue = std::move(anyFromString);
                }
            }
            // check if we are doing a safe cast between numbers
            // for instance, it is safe to use int(100) to set
            // a uint8_t port, but not int(-42) or int(300)
            if constexpr(std::is_arithmetic_v<T>) {
                if(mismatching && IsCastingSafe(previousType, rValue)) {
                    mismatching = false;
                }
            }

            if(mismatching) {
                DebugMessage();

                auto msg = util::StrCat("Blackboard::set(", rKey,
                                        "): once declared, "
                                        "the Type of a port shall not change. "
                                        "Previously declared Type [",
                                        behaviortree::Demangle(previousType), "], current Type [", behaviortree::Demangle(typeid(T)), "]");
                throw util::LogicError(msg);
            }
        }
        // if doing set<BT::Any>, skip type check
        if constexpr(std::is_same_v<Any, T>) {
            rPreviousAny = newValue;
        } else {
            // copy only if the type is compatible
            newValue.CopyInto(rPreviousAny);
        }
        rEntry.sequenceId++;
        rEntry.stamp = std::chrono::steady_clock::now().time_since_epoch();
    }
}

template<typename T>
inline bool Blackboard::Get(const std::string &rKey, T &rValue) const {
    if(auto anyLocked = GetAnyLocked(rKey)) {
        if(anyLocked.Get()->Empty()) {
            return false;
        }
        rValue = anyLocked.Get()->Cast<T>();
        return true;
    }
    return false;
}

template<typename T>
inline Expected<Timestamp> Blackboard::GetStamped(const std::string &rKey, T &rValue) const {
    if(auto entry = GetEntry(rKey)) {
        std::unique_lock lk(entry->entryMutex);
        if(entry->value.Empty()) {
            return nonstd::make_unexpected(util::StrCat("Blackboard::GetStamped() error. Entry [", rKey, "] hasn't been initialized, yet"));
        }
        rValue = entry->value.Cast<T>();
        return Timestamp{entry->sequenceId, entry->stamp};
    }
    return nonstd::make_unexpected(util::StrCat("Blackboard::GetStamped() error. Missing key [", rKey, "]"));
}

template<typename T>
inline Expected<StampedValue<T>> Blackboard::GetStamped(const std::string &rKey) const {
    StampedValue<T> out;
    if(auto res = GetStamped<T>(rKey, out.value)) {
        out.stamp = *res;
        return out;
    } else {
        return nonstd::make_unexpected(res.error());
    }
}

}// namespace behaviortree

#endif// BEHAVIORTREE_BLACKBOARD_H
