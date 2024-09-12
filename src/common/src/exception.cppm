module;

#include "common/string.hpp"

export module common.exception;

import <stdexcept>;
import <string>;

namespace util {
export class Exception: public std::exception {
 public:
    explicit Exception(std::string_view);

    template<typename... SV>
    Exception(const SV &...args): m_message(util::StrCat(args...)) {}

    const char *what() const noexcept;

 private:
    std::string m_message;
};

// This errors are usually related to problems which "probably" require code refactoring
// to be fixed.
export class LogicError: public Exception {
 public:
    explicit LogicError(std::string_view);

    template<typename... SV>
    LogicError(const SV &...args): Exception(args...) {}
};

// This errors are usually related to problems that are relted to data or conditions
// that happen only at run-time
export class RuntimeError: public Exception {
 public:
    explicit RuntimeError(std::string_view);

    template<typename... SV>
    RuntimeError(const SV &...args): Exception(args...) {}
};

}// namespace util

// module exception;
// module;
