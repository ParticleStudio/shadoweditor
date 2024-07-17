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
inline char const *DemangleAlloc(char const *pName) noexcept;
inline void DemangleFree(char const *pName) noexcept;

class ScopedDemangledName {
 private:
    char const *m_P;

 public:
    explicit ScopedDemangledName(char const *pName) noexcept: m_P(DemangleAlloc(pName)) {}

    ~ScopedDemangledName() noexcept {
        DemangleFree(m_P);
    }

    char const *Get() const noexcept {
        return m_P;
    }

    ScopedDemangledName(ScopedDemangledName const &) = delete;
    ScopedDemangledName &operator=(ScopedDemangledName const &) = delete;
};

#if defined(HAS_CXXABI_H)

inline char const *DemangleAlloc(char const *ptrName) noexcept {
    int status = 0;
    std::size_t size = 0;
    return abi::__cxa_demangle(ptrName, NULL, &size, &status);
}

inline void DemangleFree(char const *ptrName) noexcept {
    std::free(const_cast<char *>(ptrName));
}

#else

inline char const *DemangleAlloc(char const *pName) noexcept {
    return pName;
}

inline void DemangleFree(char const *) noexcept {}

inline std::string Demangle(char const *pName) {
    return pName;
}

#endif

inline std::string Demangle(const std::type_index &rIndex) {
    if(rIndex == typeid(std::string)) {
        return "std::string";
    }
    if(rIndex == typeid(std::string_view)) {
        return "std::string_view";
    }
    if(rIndex == typeid(std::chrono::seconds)) {
        return "std::chrono::seconds";
    }
    if(rIndex == typeid(std::chrono::milliseconds)) {
        return "std::chrono::milliseconds";
    }
    if(rIndex == typeid(std::chrono::microseconds)) {
        return "std::chrono::microseconds";
    }

    ScopedDemangledName demangledName(rIndex.name());
    char const *const pName = demangledName.Get();
    if(pName) {
        return pName;
    } else {
        return rIndex.name();
    }
}

inline std::string Demangle(const std::type_info &rInfo) {
    return Demangle(std::type_index(rInfo));
}

}// namespace behaviortree

#undef HAS_CXXABI_H

#endif// BEHAVIORTREE_DEMANGLE_UTIL_H
