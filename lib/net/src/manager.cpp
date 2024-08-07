#include "net/manager.h"

namespace net {
Manager::Manager(Singleton<Manager>::Token) {
}

void Manager::Init() {
}

void Manager::Release() {
}

TcpServer &Manager::NewTcpServer(asio::ip::tcp protocol, std::string &rIp, uint32_t port) {
    m_tcpServerVec.emplace_back(protocol, rIp, port);
}

TcpServer Manager::GetTcpServer(uint32_t serverId) {
    if(serverId < 0 || serverId >= m_tcpServerVec.size()) {
        m_tcpServerVec.emplace_back();
    }
    return TcpServer();
}
}// namespace net