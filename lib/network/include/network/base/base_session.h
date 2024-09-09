#ifndef NETWORK_BASE_SESSION_H
#define NETWORK_BASE_SESSION_H

#include <memory>

namespace network {
class BaseSession: public std::enable_shared_from_this<BaseSession> {
 public:
    virtual void Start();

 private:
};
}// namespace network

#endif// NETWORK_BASE_SESSION_H
