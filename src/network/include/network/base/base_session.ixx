module;

export module network.base_session;

import <memory>;

namespace network {
export class BaseSession: public std::enable_shared_from_this<BaseSession> {
 public:
    virtual void Start();

 private:
};
}// namespace network
