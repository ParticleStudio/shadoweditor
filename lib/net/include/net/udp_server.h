#ifndef NET_UDP_SERVER_H
#define NET_UDP_SERVER_H

#include <cstdint>
#include <string>

#include "net/base_server.h"

namespace net {

class UdpServer: public BaseServer {
 public:
    ~UdpServer() noexcept override;

    void Init() override;

    void Release() override;

 private:

};

}// namespace net

#endif// NET_UDP_SERVER_H
