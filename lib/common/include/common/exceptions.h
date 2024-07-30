#ifndef COMMON_EXCEPTIONS_H
#define COMMON_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include "common/strcat.hpp"

namespace util {
class Exception: public std::exception {
 public:
    Exception(std::string_view message): m_message(static_cast<std::string>(message)) {}

    template<typename... SV>
    Exception(const SV &...args): m_message(util::StrCat(args...)) {}

    const char *what() const noexcept {
        return m_message.c_str();
    }

 private:
    std::string m_message;
};

// This errors are usually related to problems which "probably" require code refactoring
// to be fixed.
class LogicError: public Exception {
 public:
    LogicError(std::string_view message): Exception(message) {}

    template<typename... SV>
    LogicError(const SV &...args): Exception(args...) {}
};

// This errors are usually related to problems that are relted to data or conditions
// that happen only at run-time
class RuntimeError: public Exception {
 public:
    RuntimeError(std::string_view message): Exception(message) {}

    template<typename... SV>
    RuntimeError(const SV &...args): Exception(args...) {}
};

}// namespace util

#endif// COMMON_EXCEPTIONS_H
