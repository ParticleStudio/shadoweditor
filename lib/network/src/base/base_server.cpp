module;

#include <cstdint>
#include <string>

module shadow.network.base;

namespace shadow::network {
BaseServer::BaseServer() {
}

BaseServer::BaseServer(uint32_t serverId, std::string &rAddress, uint32_t port): m_serverId(serverId), m_address(rAddress), m_port(port) {
}

BaseServer::~BaseServer() noexcept {
}

void BaseServer::Init() {
}

void BaseServer::Release() {
}
} // namespace shadow::network

// module shadow.network.base;
// module;
