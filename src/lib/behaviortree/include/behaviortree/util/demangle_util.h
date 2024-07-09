#ifndef BEHAVIORTREE_DEMANGLE_UTIL_H
#define BEHAVIORTREE_DEMANGLE_UTIL_H

#include <chrono>
#include <string>
#include <typeindex>

#if defined(__clang__) && defined(__has_include)
#if __has_include(<cxxabi.h>)
#define HAS_CXXABI_H
#endif
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#define HAS_CXXABI_H
#endif

#if defined(HAS_CXXABI_H)
#include <cxxabi.h>

#include <cstddef>
#include <cstdlib>
#endif

namespace behaviortree {
inline char const* DemangleAlloc(char const* ptrName) noexcept;
inline void DemangleFree(char const* name) noexcept;

class ScopedDemangledName {
 private:
    char const* m_P;

 public:
    explicit ScopedDemangledName(char const* ptrName) noexcept: m_P(DemangleAlloc(ptrName)) {}

    ~ScopedDemangledName() noexcept {
        DemangleFree(m_P);
    }

    char const* Get() const noexcept {
        return m_P;
    }

    ScopedDemangledName(ScopedDemangledName const&) = delete;
    ScopedDemangledName& operator=(ScopedDemangledName const&) = delete;
};

#if defined(HAS_CXXABI_H)

inline char const* DemangleAlloc(char const* ptrName) noexcept {
    int status = 0;
    std::size_t size = 0;
    return abi::__cxa_demangle(ptrName, NULL, &size, &status);
}

inline void DemangleFree(char const* ptrName) noexcept {
    std::free(const_cast<char*>(ptrName));
}

#else

inline char const* DemangleAlloc(char const* ptrName) noexcept {
    return ptrName;
}

inline void DemangleFree(char const*) noexcept {}

inline std::string Demangle(char const* ptrName) {
    return ptrName;
}

#endif

inline std::string Demangle(const std::type_index& refIndex) {
    if(refIndex == typeid(std::string)) {
        return "std::string";
    }
    if(refIndex == typeid(std::string_view)) {
        return "std::string_view";
    }
    if(refIndex == typeid(std::chrono::seconds)) {
        return "std::chrono::seconds";
    }
    if(refIndex == typeid(std::chrono::milliseconds)) {
        return "std::chrono::milliseconds";
    }
    if(refIndex == typeid(std::chrono::microseconds)) {
        return "std::chrono::microseconds";
    }

    ScopedDemangledName demangledName(refIndex.name());
    char const* const ptrP = demangledName.Get();
    if(ptrP) {
        return ptrP;
    } else {
        return refIndex.name();
    }
}

inline std::string Demangle(const std::type_info& refInfo) {
    return Demangle(std::type_index(refInfo));
}

}// namespace behaviortree

#undef HAS_CXXABI_H

#endif// BEHAVIORTREE_DEMANGLE_UTIL_H
