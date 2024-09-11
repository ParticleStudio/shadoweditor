#ifndef NETWORK_TCP_SERVER_H
#define NETWORK_TCP_SERVER_H

#include <cstdint>
#include <string>

#include "network/base/base_server.h"
#include "network/network_common.h"

namespace network {
class NETWORK_API TcpServer: public network::BaseServer {
 public:
    //    TcpServer(const asio::ip::tcp &, const std::string &, uint32_t port);

    ~TcpServer() noexcept override;

    void Init() override;

    void Release() override;

    void Accept();

    void Run();

 private:
    //    asio::io_context m_ioContext{};
    //    asio::ip::tcp::acceptor m_acceptor;
};
}// namespace network

#endif// NETWORK_TCP_SERVER_H
