module;

#include <string>

module shadow.exception;

namespace shadow::exception {
Exception::Exception(std::string_view message): m_message(static_cast<std::string>(message)) {
}

const char *Exception::what() const noexcept {
    return m_message.c_str();
}

LogicError::LogicError(std::string_view message): Exception(message) {

}

RuntimeError::RuntimeError(std::string_view message): Exception(message) {

}
} // namespace shadow::exception

// module shadow.exception;
// module;
