module;

module shadow.network.tcp;

namespace shadow::network {
//TcpSession::TcpSession(asio::ip::tcp::socket socket): m_socket(std::move(socket)) {
//}

void TcpSession::Start() {
    Read();
}

void TcpSession::Read() {
    //    auto self(shared_from_this());
    //    this->m_socket.async_read_some(
    //            asio::buffer(m_reciveBuffer),
    //            [this, self](asio::error_code errorCode, size_t length) {
    //                if(!errorCode) {
    //                    this->Write(length);
    //                }
    //            }
    //    );
}

void TcpSession::Write(size_t length) {
    //    auto self(shared_from_this());
    //    asio::async_write(
    //            m_socket,
    //            asio::buffer(m_sendBuffer, length),
    //            [this, self](asio::error_code errorCode, size_t length) {
    //                if(!errorCode) {
    //                    this->Read();
    //                }
    //            }
    //    );
}
} // namespace shadow::network

// module shadow.network.tcp;
// module;
