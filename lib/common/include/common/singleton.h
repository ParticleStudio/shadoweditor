#ifndef COMMON_SINGLETON_H
#define COMMON_SINGLETON_H

#include <iostream>

#include "common/define.h"

namespace common {
// 单例基类
template<class T>
class Singleton {
 public:
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

    virtual ~Singleton() noexcept = default;

    [[maybe_unused]] static T &GetInstance() noexcept(std::is_nothrow_constructible<T>::value) {
        static T s_instance{Token()};
        return s_instance;
    };

 protected:
    Singleton() noexcept = default;

    struct Token {};
};
}// namespace common

#endif// !COMMON_SINGLETON_H
