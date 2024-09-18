export module behaviortree.script_node;

import behaviortree.action_node;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
export class BEHAVIORTREE_API ScriptNode: public SyncActionNode {
 public:
    ScriptNode(const std::string &rName, const NodeConfig &rConfig): SyncActionNode(rName, rConfig) {
        SetRegistrationId("ScriptNode");
        LoadExecutor();
    }

    static PortMap ProvidedPorts() {
        return {InputPort<std::string>("code Piece of code that can be parsed")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();
        if(m_Executor) {
            Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
            m_Executor(env);
        }
        return NodeStatus::Success;
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("code", script)) {
            throw util::RuntimeError("Missing port [code] in Script");
        }
        if(script == m_Script) {
            return;
        }
        auto executor = ParseScript(script);
        if(!executor) {
            throw util::RuntimeError(executor.error());
        } else {
            m_Executor = executor.value();
            m_Script = script;
        }
    }

    std::string m_Script;
    ScriptFunction m_Executor;
};

}// namespace behaviortree

// module behaviortree.script_node;
