#include "net/manager.h"

namespace net {
Manager::Manager(Singleton<Manager>::Token) {
}

void Manager::Init() {
}

void Manager::Release() {
}

BaseServer *Manager::NewTcpServer(const asio::ip::tcp &rProtocol, const std::string &rIp, uint32_t port) {
    m_tcpServerList.emplace_back(std::move(std::make_unique<TcpServer>(rProtocol, rIp, port)));
    auto pTcpServer = m_tcpServerList.back().get();
//    pTcpServer->Run();
    return pTcpServer;
}

BaseServer *Manager::GetTcpServer(uint32_t serverId) {
    if(serverId < 0 || serverId >= m_tcpServerList.size()) {
        return nullptr;
    }
    return m_tcpServerList.back().get();
}
}// namespace net