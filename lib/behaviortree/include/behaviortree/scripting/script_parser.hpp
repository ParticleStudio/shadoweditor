#ifndef BEHAVIORTREE_SCRIPT_PARSER_HPP
#define BEHAVIORTREE_SCRIPT_PARSER_HPP

import <string>;
import <memory>;

import behaviortree.blackboard;

#include "behaviortree/contrib/expected.hpp"

namespace behaviortree {

/// Simple map (string->nt), used to Convert enums in the
/// scripting language
using EnumsTable = std::unordered_map<std::string, int>;
using EnumsTablePtr = std::shared_ptr<EnumsTable>;

namespace Ast {
/**
   * @brief The Environment class is used to encapsulate
   * the information and states needed by the scripting language
   */
struct Environment {
    behaviortree::Blackboard::Ptr ptrVars;
    EnumsTablePtr ptrEnums;
};
}// namespace Ast

/**
 * @brief ValidateScript will check if a certain string is valid.
 */
nonstd::expected<std::monostate, std::string> ValidateScript(const std::string &refScript);

using ScriptFunction = std::function<Any(Ast::Environment &refEnv)>;

nonstd::expected<ScriptFunction, std::string> ParseScript(const std::string &refScript);

nonstd::expected<Any, std::string> ParseScriptAndExecute(Ast::Environment &refEnv, const std::string &refScript);

}// namespace behaviortree

#endif// BEHAVIORTREE_SCRIPT_PARSER_HPP
