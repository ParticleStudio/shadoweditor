#include "net/tcp_server.h"

namespace net {
TcpServer::TcpServer(asio::ip::tcp protocol, std::string &rIp, uint32_t port): m_acceptor(m_service, protocol, port) {

}

TcpServer::~TcpServer() {
}

void TcpServer::Init() {
}

void TcpServer::Release() {
}

}// namespace net