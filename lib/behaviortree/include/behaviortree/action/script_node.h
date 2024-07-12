#ifndef BEHAVIORTREE_SCRIPT_NODE_H
#define BEHAVIORTREE_SCRIPT_NODE_H

#include "behaviortree/action_node.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
class ScriptNode: public SyncActionNode {
 public:
    ScriptNode(const std::string &refName, const NodeConfig &refConfig): SyncActionNode(refName, refConfig) {
        SetRegistrationId("ScriptNode");

        LoadExecutor();
    }

    static PortsList ProvidedPorts() {
        return {InputPort<std::string>(
                "code", "Piece of code that can be parsed"
        )};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();
        if(m_Executor) {
            Ast::Environment env = {
                    GetConfig().ptrBlackboard, GetConfig().ptrEnums
            };
            m_Executor(env);
        }
        return NodeStatus::SUCCESS;
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("code", script)) {
            throw RuntimeError("Missing port [code] in Script");
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

#endif// BEHAVIORTREE_SCRIPT_NODE_H
