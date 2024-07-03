#ifndef BEHAVIORTREE_SCRIPT_PRECONDITION_H
#define BEHAVIORTREE_SCRIPT_PRECONDITION_H

#include <type_traits>

#include "behaviortree/decorator_node.h"
#include "behaviortree/scripting/script_parser.hpp"

namespace behaviortree {
class PreconditionNode: public DecoratorNode {
 public:
    PreconditionNode(const std::string& refName, const NodeConfig& refConfig)
        : DecoratorNode(refName, refConfig) {
        LoadExecutor();
    }

    virtual ~PreconditionNode() override = default;

    static PortsList ProvidedPorts() {
        return {InputPort<std::string>("if"),
                InputPort<NodeStatus>("else", NodeStatus::FAILURE,
                                      "Return status if condition is "
                                      "false")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        LoadExecutor();

        behaviortree::NodeStatus elseReturn;
        if(!GetInput("else", elseReturn)) {
            throw RuntimeError("Missing parameter [else] in Precondition");
        }

        Ast::Environment env = {Config().blackboard, Config().enums};
        if(_executor(env).cast<bool>()) {
            auto const childStatus = m_ChildNode->ExecuteTick();
            if(isStatusCompleted(childStatus)) {
                ResetChild();
            }
            return childStatus;
        } else {
            return elseReturn;
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
