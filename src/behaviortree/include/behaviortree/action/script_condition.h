#ifndef BEHAVIORTREE_SCRIPT_CONDITION_H
#define BEHAVIORTREE_SCRIPT_CONDITION_H

#include "behaviortree/condition_node.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
/**
 * @brief Execute a script, and if the result is true, return
 * SUCCESS, FAILURE otherwise.
 */
class ScriptCondition: public ConditionNode {
 public:
    ScriptCondition(const std::string& refName, const NodeConfig& refConfig)
        : ConditionNode(refName, refConfig) {
        SetRegistrationID("ScriptCondition");
        LoadExecutor();
    }

    static PortsList ProvidedPorts() {
        return {InputPort("code",
                          "Piece of code that can be parsed. Must return false or "
                          "true")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();

        Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};
        auto result = m_Executor(env);
        return (result.Cast<bool>()) ? NodeStatus::SUCCESS : NodeStatus::FAILURE;
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("code", script)) {
            throw RuntimeError("Missing port [code] in ScriptCondition");
        }
        if(script == m_Script) {
            return;
        }
        auto executor = ParseScript(script);
        if(!executor) {
            throw RuntimeError(executor.error());
        } else {
            m_Executor = executor.value();
            m_Script = script;
        }
    }

    std::string m_Script;
    ScriptFunction m_Executor;
};

}// namespace behaviortree

#endif// BEHAVIORTREE_SCRIPT_CONDITION_H
