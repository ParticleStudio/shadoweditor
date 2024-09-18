export module behaviortree.script_precondition;

import <type_traits>;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
export class BEHAVIORTREE_API PreconditionNode: public DecoratorNode {
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
            throw util::RuntimeError("Missing parameter [else] in Precondition");
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
            throw util::RuntimeError("Missing parameter [if] in Precondition");
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

// module behaviortree.script_precondition;
