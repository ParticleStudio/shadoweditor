#include "net/tcp_server.h"

namespace net {
TcpServer::TcpServer(asio::ip::tcp &rProtocol, std::string &rIp, uint32_t port): m_acceptor(m_service, rProtocol, port) {

}

TcpServer::~TcpServer() {
}

void TcpServer::Init() {
}

void TcpServer::Release() {
}

}// namespace net