#ifndef NET_TCP_SESSION_H
#define NET_TCP_SESSION_H

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "net/base/base_session.h"

namespace net {
class TcpSession: public net::BaseSession {
 public:
//    explicit TcpSession(asio::ip::tcp::socket socket);

    void Start() override;

    void Read();

    void Write(std::size_t);

 private:
//    asio::ip::tcp::socket m_socket;
    std::vector<char> m_reciveBuffer;
    std::vector<char> m_sendBuffer;
};
}// namespace net

#endif//NET_TCP_SESSION_H
