#ifndef BEHAVIORTREE_BLACKBOARD_H
#define BEHAVIORTREE_BLACKBOARD_H

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "behaviortree/basic_types.h"
#include "behaviortree/contrib/json.hpp"
#include "behaviortree/exceptions.h"
#include "behaviortree/util/locked_reference.hpp"
#include "behaviortree/util/safe_any.hpp"

namespace behaviortree {

/// This type contains a pointer to Any, protected
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
    Blackboard(Blackboard::Ptr ptrParentBlackboard): m_ParentBlackboard(ptrParentBlackboard) {}

 public:
    struct Entry {
        Any value;
        TypeInfo typeInfo;
        StringConverter stringConverter;
        mutable std::mutex entryMutex;

        uint64_t sequenceId{0};
        // timestamp since epoch
        std::chrono::nanoseconds stamp{std::chrono::nanoseconds{0}};

        Entry(const TypeInfo& refTypeInfo): typeInfo(refTypeInfo) {}

        Entry& operator=(const Entry& refOther);
    };

    /** Use this static method to create an instance of the BlackBoard
    *   to share among all your NodeTrees.
    */
    static Blackboard::Ptr Create(Blackboard::Ptr ptrParentBlackboard = {}) {
        return std::shared_ptr<Blackboard>(new Blackboard(ptrParentBlackboard));
    }

    virtual ~Blackboard() = default;

    void EnableAutoRemapping(bool remapping);

    [[nodiscard]] const std::shared_ptr<Entry> GetEntry(const std::string& refKey) const;

    [[nodiscard]] std::shared_ptr<Blackboard::Entry> GetEntry(const std::string& refKey);

    [[nodiscard]] AnyPtrLocked GetAnyLocked(const std::string& refKey);

    [[nodiscard]] AnyPtrLocked GetAnyLocked(const std::string& refKey) const;

    [[deprecated("Use getAnyLocked instead")]] const Any*
    GetAny(const std::string& key) const;

    [[deprecated("Use getAnyLocked instead")]] Any* GetAny(const std::string& key);

    /** Return true if the entry with the given key was found.
   *  Note that this method may throw an exception if the cast to T failed.
   */
    template<typename T>
    [[nodiscard]] bool Get(const std::string& key, T& value) const;

    template<typename T>
    [[nodiscard]] Expected<Timestamp> GetStamped(const std::string& refKey, T& refValue) const;

    /**
   * Version of get() that throws if it fails.
   */
    template<typename T>
    [[nodiscard]] T Get(const std::string& key) const;

    template<typename T>
    [[nodiscard]] Expected<StampedValue<T>> GetStamped(const std::string& refKey) const;

    /// Update the entry with the given key
    template<typename T>
    void Set(const std::string& refKey, const T& refValue);

    void Unset(const std::string& refKey);

    [[nodiscard]] const TypeInfo* EntryInfo(const std::string& refKey);

    void AddSubtreeRemapping(StringView internal, StringView external);

    void DebugMessage() const;

    [[nodiscard]] std::vector<StringView> GetKeys() const;

    [[deprecated("This command is unsafe. Consider using Backup/Restore instead")]] void
    Clear();

    [[deprecated("Use getAnyLocked to access safely an Entry")]] std::recursive_mutex&
    EntryMutex() const;

    void CreateEntry(const std::string& refKey, const TypeInfo& refTypeInfo);

    /**
   * @brief cloneInto copies the values of the entries
   * into another blackboard.  Known limitations:
   *
   * - it doesn't update the remapping in dst
   * - it doesn't change the parent blackboard os dst
   *
   * @param refDst destination, i.e. blackboard to be updated
   */
    void CloneInto(Blackboard& refDst) const;

    Blackboard::Ptr Parent();

    // recursively look for parent Blackboard, until you find the root
    Blackboard* RootBlackboard();

    const Blackboard* RootBlackboard() const;

 private:
    mutable std::mutex m_Mutex;
    mutable std::recursive_mutex m_EntryMutex;
    std::unordered_map<std::string, std::shared_ptr<Entry>> m_Storage;
    std::weak_ptr<Blackboard> m_ParentBlackboard;
    std::unordered_map<std::string, std::string> m_InternalToExternal;

    std::shared_ptr<Entry> CreateEntryImpl(const std::string& refKey, const TypeInfo& refTypeInfo);

    bool m_AutoRemapping{false};
};

/**
 * @brief ExportBlackboardToJSON will create a JSON
 * that contains the current values of the blackboard.
 * Complex types must be registered with JsonExporter::get()
 */
nlohmann::json ExportBlackboardToJSON(const Blackboard& refBlackboard);

/**
 * @brief ImportBlackboardFromJSON will append elements to the blackboard,
 * using the values parsed from the JSON file created using ExportBlackboardToJSON.
 * Complex types must be registered with JsonExporter::get()
 */
void ImportBlackboardFromJSON(const nlohmann::json& refJson, Blackboard& refBlackboard);

//------------------------------------------------------

template<typename T>
inline T Blackboard::Get(const std::string& refKey) const {
    if(auto anyLocked = GetAnyLocked(refKey)) {
        const auto& refAny = anyLocked.Get();
        if(refAny->empty()) {
            throw RuntimeError("Blackboard::Get() error. Entry [", refKey,
                               "] hasn't been initialized, yet");
        }
        return anyLocked.Get()->cast<T>();
    }
    throw RuntimeError("Blackboard::Get() error. Missing key [", refKey, "]");
}

inline void Blackboard::Unset(const std::string& refKey) {
    std::unique_lock lock(m_Mutex);

    // check local storage
    auto it = m_Storage.find(refKey);
    if(it == m_Storage.end()) {
        // No entry, nothing to do.
        return;
    }

    m_Storage.erase(it);
}

template<typename T>
inline void Blackboard::Set(const std::string& refKey, const T& refValue) {
    if(StartWith(refKey, '@')) {
        RootBlackboard()->Set(refKey.substr(1, refKey.size() - 1), refValue);
        return;
    }
    std::unique_lock lock(m_Mutex);

    // check local storage
    auto it = m_Storage.find(refKey);
    Any newValue(refValue);
    if(it == m_Storage.end()) {
        lock.unlock();
        std::shared_ptr<Blackboard::Entry> entry;
        // if a new generic port is created with a string, it's type should be AnyTypeAllowed
        if constexpr(std::is_same_v<std::string, T>) {
            entry = createEntryImpl(refKey, PortInfo(PortDirection::INOUT));
        } else {
            PortInfo newPort(PortDirection::INOUT, new_value.type(),
                              GetAnyFromStringFunctor<T>());
            entry = createEntryImpl(refKey, newPort);
        }
        lock.lock();

        entry->value = newValue;
        entry->sequenceId++;
        entry->stamp = std::chrono::steady_clock::now().time_since_epoch();
    } else {
        // this is not the first time we set this entry, we need to check
        // if the type is the same or not.
        Entry& entry = *it->second;
        std::scoped_lock scoped_lock(entry.entryMutex);

        Any& previousAny = entry.value;
        // special case: entry exists but it is not strongly typed... yet
        if(!entry.typeInfo.IsStronglyTyped()) {
            // Use the new type to create a new entry that is strongly typed.
            entry.typeInfo = TypeInfo::Create<T>();
            entry.sequenceId++;
            entry.stamp = std::chrono::steady_clock::now().time_since_epoch();
            previousAny = std::move(newValue);
            return;
        }

        std::type_index previousType = entry.typeInfo.type();

        // check type mismatch
        if(previousType != std::type_index(typeid(T)) && previousType != newValue.type()) {
            bool mismatching = true;
            if(std::is_constructible<StringView, T>::value) {
                Any anyFromString = entry.typeInfo.parseString(refValue);
                if(anyFromString.empty() == false) {
                    mismatching = false;
                    newValue = std::move(anyFromString);
                }
            }
            // check if we are doing a safe cast between numbers
            // for instance, it is safe to use int(100) to set
            // a uint8_t port, but not int(-42) or int(300)
            if constexpr(std::is_arithmetic_v<T>) {
                if(mismatching && isCastingSafe(previousType, refValue)) {
                    mismatching = false;
                }
            }

            if(mismatching) {
                DebugMessage();

                auto msg = StrCat("Blackboard::set(", refKey,
                                  "): once declared, "
                                  "the type of a port shall not change. "
                                  "Previously declared type [",
                                  behaviortree::Demangle(previousType), "], current type [",
                                  behaviortree::Demangle(typeid(T)), "]");
                throw LogicError(msg);
            }
        }
        // if doing set<BT::Any>, skip type check
        if constexpr(std::is_same_v<Any, T>) {
            previousAny = newValue;
        } else {
            // copy only if the type is compatible
            newValue.CopyInto(previousAny);
        }
        entry.sequenceId++;
        entry.stamp = std::chrono::steady_clock::now().time_since_epoch();
    }
}

template<typename T>
inline bool Blackboard::Get(const std::string& refKey, T& refValue) const {
    if(auto anyLocked = GetAnyLocked(refKey)) {
        if(anyLocked.Get()->empty()) {
            return false;
        }
        refValue = anyLocked.Get()->cast<T>();
        return true;
    }
    return false;
}

template<typename T>
inline Expected<Timestamp> Blackboard::GetStamped(const std::string& key, T& value) const {
    if(auto entry = GetEntry(key)) {
        std::unique_lock lk(entry->entryMutex);
        if(entry->value.empty()) {
            return nonstd::make_unexpected(StrCat("Blackboard::GetStamped() error. Entry [",
                                                  key, "] hasn't been initialized, yet"));
        }
        value = entry->value.cast<T>();
        return Timestamp{entry->sequenceId, entry->stamp};
    }
    return nonstd::make_unexpected(
            StrCat("Blackboard::GetStamped() error. Missing key [", key, "]"));
}

template<typename T>
inline Expected<StampedValue<T>> Blackboard::GetStamped(const std::string& refKey) const {
    StampedValue<T> out;
    if(auto res = GetStamped<T>(refKey, out.value)) {
        out.stamp = *res;
        return out;
    } else {
        return nonstd::make_unexpected(res.error());
    }
}

}// namespace behaviortree

#endif// BEHAVIORTREE_BLACKBOARD_H
