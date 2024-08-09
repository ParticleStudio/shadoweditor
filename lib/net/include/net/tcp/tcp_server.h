#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include <cstdint>
#include <string>

#include "asio.hpp"
#include "net/base/base_server.h"

namespace net {
class TcpServer: public net::BaseServer {
 public:
    TcpServer(const asio::ip::tcp &, const std::string &, uint32_t port);

    ~TcpServer() noexcept override;

    void Init() override;

    void Release() override;

    void Accept();

    void Run();

 private:
    asio::io_context m_ioContext{};
    asio::ip::tcp::acceptor m_acceptor;
};
}// namespace net

#endif// NET_TCP_SERVER_H
