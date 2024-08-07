#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include <cstdint>
#include <string>

#include "net/base_server.h"

namespace net {
class TcpServer: public BaseServer {
 public:
    TcpServer(asio::ip::tcp &, std::string &, uint32_t port);

    ~TcpServer() noexcept override;

    void Init() override;

    void Release() override;

 private:
    asio::io_service m_service{};
    asio::ip::tcp::acceptor m_acceptor;
};

}// namespace net

#endif// NET_TCP_SERVER_H
