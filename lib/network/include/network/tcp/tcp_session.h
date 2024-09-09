#ifndef NETWORK_TCP_SESSION_H
#define NETWORK_TCP_SESSION_H

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "network/base/base_session.h"

namespace network {
class TcpSession: public network::BaseSession {
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
}// namespace network

#endif// NETWORK_TCP_SESSION_H
