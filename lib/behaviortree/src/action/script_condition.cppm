module;

export module behaviortree.script_condition;

import common.exception;

#include "behaviortree/common.h"
#include "behaviortree/condition_node.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
/**
 * @brief Execute a script, and if the result is true, return
 * SUCCESS, FAILURE otherwise.
 */
export class BEHAVIORTREE_API ScriptCondition: public ConditionNode {
 public:
    ScriptCondition(const std::string &rName, const NodeConfig &rConfig): ConditionNode(rName, rConfig) {
        SetRegistrationId("ScriptCondition");
        LoadExecutor();
    }

    static PortMap ProvidedPorts() {
        return {InputPort("code", "Piece of code that can be parsed. Must return false or true")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();

        Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
        auto result = m_executor(env);
        return (result.Cast<bool>()) ? NodeStatus::Success
                                     : NodeStatus::Failure;
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("code", script)) {
            throw util::RuntimeError("Missing port [code] in ScriptCondition");
        }
        if(script == m_script) {
            return;
        }
        auto executor = ParseScript(script);
        if(!executor) {
            throw util::RuntimeError(executor.error());
        } else {
            m_executor = executor.value();
            m_script = script;
        }
    }

    std::string m_script;
    ScriptFunction m_executor;
};

}// namespace behaviortree

// module behaviortree.script_condition;
// module;
