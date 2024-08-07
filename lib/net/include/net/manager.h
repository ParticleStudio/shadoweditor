#ifndef NET_MANAGER_H
#define NET_MANAGER_H

#include <cstdint>
#include <string>

#include "asio.hpp"
#include "common/singleton.h"
#include "net/tcp_server.h"
#include "net/udp_server.h"

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

    TcpServer *NewTcpServer(asio::ip::tcp &, std::string &, uint32_t);

    TcpServer *GetTcpServer(uint32_t serverId);

 private:
    std::mutex m_mutex{};
    std::list<std::unique_ptr<TcpServer>> m_tcpServerList{};
    std::list<std::unique_ptr<UdpServer>> m_udpServerList{};
};
}// namespace net

#endif// NET_MANAGER_H
