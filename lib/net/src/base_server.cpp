#include "net/base_server.h"

namespace net {
BaseServer::BaseServer() {
}

BaseServer::BaseServer(std::string &rAddress, uint32_t port): m_address(rAddress), m_port(port) {
}

BaseServer::~BaseServer() {
}

void BaseServer::Init() {
}

void BaseServer::Release() {
}

}// namespace net
