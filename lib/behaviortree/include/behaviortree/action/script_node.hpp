#ifndef BEHAVIORTREE_SCRIPT_NODE_HPP
#define BEHAVIORTREE_SCRIPT_NODE_HPP

import common.exception;

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
class BEHAVIORTREE_API ScriptNode: public SyncActionNode {
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
        if(m_executor) {
            Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
            m_executor(env);
        }
        return NodeStatus::Success;
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("code", script)) {
            throw util::RuntimeError("Missing port [code] in Script");
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

#endif// BEHAVIORTREE_SCRIPT_NODE_HPP
