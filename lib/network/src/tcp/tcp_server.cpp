module;

module shadow.network.tcp;

namespace shadow::network {
//TcpServer::TcpServer(const asio::ip::tcp &rProtocol, const std::string &rIp, uint32_t port): m_acceptor(m_ioContext, rProtocol, port) {
//    Accept();
//}

TcpServer::~TcpServer() noexcept {
}

void TcpServer::Init() {
}

void TcpServer::Release() {
}

void TcpServer::Accept() {
    //    this->m_acceptor.async_accept([this](asio::error_code errorCode, asio::ip::tcp::socket socket) {
    //        if(!errorCode) {
    //            std::make_shared<net::TcpSession>(std::move(socket))->Start();
    //        }
    //        Accept();
    //    });
}

void TcpServer::Run() {
    //    this->m_ioContext.run();
}

} // namespace shadow::network

// module shadow.network.tcp;
// module;
