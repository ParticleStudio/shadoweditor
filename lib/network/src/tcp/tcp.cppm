module;

#include <vector>

export module shadow.network.tcp;

import shadow.network.base;

#include "network/common.h"

namespace shadow::network {
export class NETWORK_API TcpServer: public shadow::network::BaseServer {
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
} // namespace shadow::network

// module shadow.network.tcp;
// module;
