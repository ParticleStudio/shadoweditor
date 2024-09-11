module;

export module network.tcp_server;

import network.base_server;

import <cstdint>;
import <string>;

#include "network/network_common.h"

namespace network {
export class NETWORK_API TcpServer: public network::BaseServer {
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
