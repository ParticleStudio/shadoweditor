module network.base_server;

namespace network {
BaseServer::BaseServer() {
}

BaseServer::BaseServer(uint32_t serverId, std::string &rAddress, uint32_t port): m_serverId(serverId), m_address(rAddress), m_port(port) {
}

BaseServer::~BaseServer() {
}

void BaseServer::Init() {
}

void BaseServer::Release() {
}
}// namespace network
