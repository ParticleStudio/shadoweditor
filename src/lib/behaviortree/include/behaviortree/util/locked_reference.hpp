#ifndef BEHAVIORTREE_LOCKED_REFERENCE_HPP
#define BEHAVIORTREE_LOCKED_REFERENCE_HPP

#include <mutex>

#include "behaviortree/util/safe_any.hpp"

namespace behaviortree {
/**
 * @brief The LockedPtr class is used to share a pointer to an object
 * and a mutex that protects the read/write access to that object.
 *
 * As long as the object remains in scope, the mutex is locked, therefore
 * you must destroy this instance as soon as the pointer was used.
 */
template<typename T>
class LockedPtr {
 public:
    LockedPtr() = default;

    LockedPtr(T* obj, std::mutex* ptrObjMutex): m_Ref(obj), m_Mutex(ptrObjMutex) {
        m_Mutex->lock();
    }

    ~LockedPtr() {
        if(m_Mutex) {
            m_Mutex->unlock();
        }
    }

    LockedPtr(LockedPtr const&) = delete;
    LockedPtr& operator=(LockedPtr const&) = delete;

    LockedPtr(LockedPtr&& refOther) {
        std::swap(m_Ref, refOther.m_Ref);
        std::swap(m_Mutex, refOther.m_Mutex);
    }

    LockedPtr& operator=(LockedPtr&& refOther) {
        std::swap(m_Ref, refOther.m_Ref);
        std::swap(m_Mutex, refOther.m_Mutex);
    }

    operator bool() const {
        return m_Ref != nullptr;
    }

    void Lock() {
        if(m_Mutex) {
            m_Mutex->lock();
        }
    }

    void Unlock() {
        if(m_Mutex) {
            m_Mutex->unlock();
        }
    }

    const T* Get() const {
        return m_Ref;
    }

    const T* operator->() const {
        return m_Ref;
    }

    T* operator->() {
        return m_Ref;
    }

    template<typename OtherT>
    void Assign(const OtherT& refOther) {
        if(m_Ref == nullptr) {
            throw std::runtime_error("Empty LockedPtr reference");
        } else if constexpr(std::is_same_v<T, OtherT>) {
            *m_Ref = refOther;
        } else if constexpr(std::is_same_v<behaviortree::Any, OtherT>) {
            refOther->CopyInto(*m_Ref);
        } else {
            *m_Ref = T(refOther);
        }
    }

 private:
    T* m_Ref{nullptr};
    std::mutex* m_Mutex{nullptr};
};

}// namespace behaviortree

#endif// BEHAVIORTREE_LOCKED_REFERENCE_HPP
