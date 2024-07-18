#ifndef BEHAVIORTREE_EXCEPTIONS_H
#define BEHAVIORTREE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include "behaviortree/define.h"
#include "util/strcat.hpp"

namespace behaviortree {
class BehaviorTreeException: public std::exception {
 public:
    BehaviorTreeException(std::string_view message): m_message(static_cast<std::string>(message)) {}

    template<typename... SV>
    BehaviorTreeException(const SV &...args): m_message(StrCat(args...)) {}

    const char *what() const noexcept {
        return m_message.c_str();
    }

 private:
    std::string m_message;
};

// This errors are usually related to problems which "probably" require code refactoring
// to be fixed.
class LogicError: public BehaviorTreeException {
 public:
    LogicError(std::string_view message): BehaviorTreeException(message) {}

    template<typename... SV>
    LogicError(const SV &...args): BehaviorTreeException(args...) {}
};

// This errors are usually related to problems that are relted to data or conditions
// that happen only at run-time
class RuntimeError: public BehaviorTreeException {
 public:
    RuntimeError(std::string_view message): BehaviorTreeException(message) {}

    template<typename... SV>
    RuntimeError(const SV &...args): BehaviorTreeException(args...) {}
};

}// namespace behaviortree

#endif// BEHAVIORTREE_EXCEPTIONS_H
