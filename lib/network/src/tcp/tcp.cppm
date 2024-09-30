module;

export module network.tcp;

import network.base;

import <cstdint>;
import <string>;
import <vector>;

#include "network/common.h"

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

export class TcpSession: public network::BaseSession {
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
} // namespace network

// module network.tcp;
// module;
