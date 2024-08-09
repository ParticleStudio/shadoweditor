#include "net/tcp/tcp_server.h"

#include "asio.hpp"
#include "net/tcp/tcp_session.h"

namespace net {
TcpServer::TcpServer(const asio::ip::tcp &rProtocol, const std::string &rIp, uint32_t port): m_acceptor(m_ioContext, rProtocol, port) {
    Accept();
}

TcpServer::~TcpServer() {
}

void TcpServer::Init() {
}

void TcpServer::Release() {
}

void TcpServer::Accept() {
    this->m_acceptor.async_accept([this](asio::error_code errorCode, asio::ip::tcp::socket socket) {
        if(!errorCode) {
            std::make_shared<net::TcpSession>(std::move(socket))->Start();
        }
        Accept();
    });
}

void TcpServer::Run() {
    this->m_ioContext.run();
}

}// namespace net