export module behaviortree.subtree_node;

import <string>;

import behaviortree.decorator_node;

#include "behaviortree/behaviortree_common.h"

namespace behaviortree {
/**
 * @brief The SubtreeNode is a way to wrap an entire Subtree,
 * creating a separated BlackBoard.
 * If you want to have data flow through ports, you need to explicitly
 * remap the ports.
 *
 * NOTE: _autoremap will exclude all the ports which name start with underscore '_'
 *
 * Consider this example:

<root main_tree_to_execute = "MainTree" >

    <BehaviorTree ID="MainTree">
        <Sequence>

        <Script code="myParam='Hello'" />
        <Subtree ID="Talk" param="{myParam}" />

        <Subtree ID="Talk" param="World" />

        <Script code="param='Auto remapped'" />
        <Subtree ID="Talk" _autoremap="1"  />

        </Sequence>
    </BehaviorTree>

    <BehaviorTree ID="Talk">
        <SaySomething message="{param}" />
    </BehaviorTree>
</root>

 * You may notice three different approaches to remapping:
 *
 * 1) Subtree: "{param}"  -> Parent: "{myParam}" -> Value: "Hello"
 *    Classical remapping from one port to another, but you need to use the syntax
 *    {myParam} to say that you are remapping the another port.
 *
 * 2) Subtree: "{param}" -> Value: "World"
 *    syntax without {}, in this case param directly point to the __string__ "World".
 *
 * 3) Subtree: "{param}" -> Parent: "{parent}"
 *    Setting to true (or 1) the attribute "_autoremap", we are automatically remapping
 *    each port. Useful to avoid boilerplate.
 */
export class BEHAVIORTREE_API SubtreeNode: public DecoratorNode {
 public:
    SubtreeNode(const std::string &rName, const NodeConfig &rConfig);

    virtual ~SubtreeNode() override = default;

    static PortMap ProvidedPorts();

    void SetSubtreeId(const std::string &rSubtreeId) {
        m_SubtreeId = rSubtreeId;
    }

    const std::string &GetSubtreeId() const {
        return m_SubtreeId;
    }
    virtual behaviortree::NodeStatus Tick() override;

    virtual NodeType Type() const override final {
        return NodeType::Subtree;
    }

 private:
    std::string m_SubtreeId;
};

}// namespace behaviortree

// module behaviortree.subtree_node;
