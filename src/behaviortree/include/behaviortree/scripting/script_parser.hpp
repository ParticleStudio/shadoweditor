#ifndef BEHAVIORTREE_SCRIPT_PARSER_HPP
#define BEHAVIORTREE_SCRIPT_PARSER_HPP

#include "behaviortree/blackboard.h"

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
    behaviortree::Blackboard::Ptr vars;
    EnumsTablePtr enums;
};
}// namespace Ast

/**
 * @brief ValidateScript will check if a certain string is valid.
 */
Result ValidateScript(const std::string& refScript);

using ScriptFunction = std::function<Any(Ast::Environment& refEnv)>;

Expected<ScriptFunction> ParseScript(const std::string& refScript);

Expected<Any> ParseScriptAndExecute(Ast::Environment& refEnv, const std::string& refScript);

}// namespace behaviortree

#endif// BEHAVIORTREE_SCRIPT_PARSER_HPP
