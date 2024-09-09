#ifndef NETWORK_BASE_SERVER_H
#define NETWORK_BASE_SERVER_H

#include <cstdio>
#include <iostream>
#include <string>

namespace network {
class BaseServer {
 public:
    BaseServer();

    BaseServer(uint32_t, std::string &, uint32_t);

    virtual ~BaseServer() noexcept;

    virtual void Init();

    virtual void Release();

 private:
    uint32_t m_serverId{0};
    std::string m_address{"127.0.0.1"};
    uint32_t m_port{0};
};
}// namespace network

#endif// NETWORK_BASE_SERVER_H
