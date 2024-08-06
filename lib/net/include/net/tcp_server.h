#ifndef NET_TCP_SERVER_H
#define NET_TCP_SERVER_H

#include "net/base_server.h"

namespace net {

class TcpServer : public BaseServer {
 public:
    void virtual Init() override;

 private:

};

}// namespace net

#endif// NET_TCP_SERVER_H
