#ifndef NET_BASE_SESSION_H
#define NET_BASE_SESSION_H

#include <memory>

namespace net{
class BaseSession: public std::enable_shared_from_this<BaseSession> {
 public:
    virtual void Start();

 private:
};
}

#endif//NET_BASE_SESSION_H
