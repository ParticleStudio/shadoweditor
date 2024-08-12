#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <cstdint>
#include <mutex>
#include <string>

#include "asio.hpp"
#include "common/singleton.h"
#include "net/tcp/tcp_server.h"

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

    BaseServer *NewTcpServer(const asio::ip::tcp &, const std::string &, uint32_t);

    BaseServer *GetTcpServer(uint32_t serverId);

 private:
    std::mutex m_mutex{};
    std::list<std::unique_ptr<net::BaseServer>> m_tcpServerList{};
};
}// namespace net

#endif// NET_MANAGER_H
