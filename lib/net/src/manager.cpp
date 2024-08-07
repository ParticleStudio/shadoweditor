#include "net/manager.h"

namespace net {
Manager::Manager(Singleton<Manager>::Token) {
}

void Manager::Init() {
}

void Manager::Release() {
}

TcpServer *Manager::NewTcpServer(asio::ip::tcp &rProtocol, std::string &rIp, uint32_t port) {
    m_tcpServerList.emplace_back(rProtocol, rIp, port);
    return m_tcpServerList.back().get();
}

TcpServer *Manager::GetTcpServer(uint32_t serverId) {
    if(serverId < 0 || serverId >= m_tcpServerList.size()) {
        return nullptr;
    }
    return m_tcpServerList.
}
}// namespace net