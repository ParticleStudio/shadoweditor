module;

#include <cstdint>
#include <iostream>
#include <string>

export module shadow.network.base;

namespace shadow::network {
export class BaseServer {
 public:
    BaseServer();

    BaseServer(uint32_t, std::string &, uint32_t);

    virtual ~BaseServer() noexcept;

    virtual void Init();

    virtual void Release();

 private:
    uint32_t m_serverId{0};
    std::string m_address{"127.0.0.1"};
    uint32_t m_port{0};
};

export class BaseSession: public std::enable_shared_from_this<BaseSession> {
 public:
    virtual ~BaseSession() = default;
    virtual void Start();

 private:
};
} // namespace shadow::network

// module shadow.network.base;
// module;
