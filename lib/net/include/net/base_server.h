#ifndef NET_BASE_SERVER_H
#define NET_BASE_SERVER_H

#include <cstdio>
#include <iostream>

#include "asio.hpp"

namespace net {
class BaseServer {
 public:
    BaseServer() = delete;

    BaseServer(std::string &, uint32_t);

    virtual ~BaseServer() noexcept;

    virtual void Init();

    virtual void Release();

 private:
    std::string m_address{"127.0.0.1"};
    uint32_t m_port{0};
};
}// namespace net

#endif// NET_BASE_SERVER_H
