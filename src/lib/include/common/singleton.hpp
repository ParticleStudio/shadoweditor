#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

#include <iostream>

# ifdef SHARED_LIB
#   ifdef WIN32
#      ifdef DLLEXPORT
#        define SINGLETON_API __declspec(dllexport)
#      else
#        define SINGLETON_API __declspec(dllimport)
#      endif // !DLLEXPORT
#   else
#     define SINGLETON_API
#   endif // !WIN32
# else
#    define SINGLETON_API
# endif // !SHARED_LIB

namespace common {
// 单例基类
template<class T>
class Singleton {
 public:
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

    virtual ~Singleton() noexcept = default;

    [[maybe_unused]] static T &GetInstance() noexcept(std::is_nothrow_constructible<T>::value) {
        static T s_Instance{Token()};
        return s_Instance;
    };

 protected:
    Singleton() noexcept = default;

    struct Token {};
};
}// namespace common

#endif // !COMMON_SINGLETON_H
