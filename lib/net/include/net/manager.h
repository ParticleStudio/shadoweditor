#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include "common/singleton.h"
#include "asio.hpp"

namespace net {
class Manager final: public common::Singleton<Manager> {
 public:
    Manager() = delete;

    Manager(const Manager &) = delete;

    Manager &operator=(const Manager &) = delete;

    explicit Manager(Singleton<Manager>::Token);

    ~Manager() noexcept override = default;

    void Init();

    void Release();

 private:
};
}// namespace net

#endif// NET_MANAGER_H
