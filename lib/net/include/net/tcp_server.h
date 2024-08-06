#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include <cstdint>
#include <string>

#include "net/base_server.h"

namespace net {

class TcpServer: public BaseServer {
 public:
    ~TcpServer() noexcept override;

    void Init() override;

    void Release() override;

 private:
};

}// namespace net

#endif// NET_TCP_SERVER_H
