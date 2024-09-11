module;

export module network.tcp_session;

import network.base_session;

import <cstdio>;
import <iostream>;
import <string>;
import <vector>;

namespace network {
export class TcpSession: public network::BaseSession {
 public:
    //    explicit TcpSession(asio::ip::tcp::socket socket);

    void Start() override;

    void Read();

    void Write(std::size_t);

 private:
    //    asio::ip::tcp::socket m_socket;
    std::vector<char> m_reciveBuffer;
    std::vector<char> m_sendBuffer;
};
}// namespace network
