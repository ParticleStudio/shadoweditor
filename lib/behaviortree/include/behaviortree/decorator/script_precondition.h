#ifndef BEHAVIORTREE_SCRIPT_PRECONDITION_H
#define BEHAVIORTREE_SCRIPT_PRECONDITION_H

#include <type_traits>

#include "behaviortree/decorator_node.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
class PreconditionNode: public DecoratorNode {
 public:
    PreconditionNode(const std::string &rName, const NodeConfig &rConfig): DecoratorNode(rName, rConfig) {
        LoadExecutor();
    }

    virtual ~PreconditionNode() override = default;

    static PortMap ProvidedPorts() {
        return {
                InputPort<std::string>("if"),
                InputPort<NodeStatus>("else", NodeStatus::Failure, "Return status if condition is false")
        };
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();

        behaviortree::NodeStatus nodeStatus;
        if(!GetInput("else", nodeStatus)) {
            throw RuntimeError("Missing parameter [else] in Precondition");
        }

        Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
        if(m_Executor(env).Cast<bool>()) {
            auto const childNodeStatus = m_childNode->ExecuteTick();
            if(IsNodeStatusCompleted(childNodeStatus)) {
                ResetChildNode();
            }
            return childNodeStatus;
        } else {
            return nodeStatus;
        }
    }

    void LoadExecutor() {
        std::string script;
        if(!GetInput("if", script)) {
            throw RuntimeError("Missing parameter [if] in Precondition");
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

#endif// BEHAVIORTREE_SCRIPT_PRECONDITION_H
