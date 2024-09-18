module;

// base
export module behaviortree;
export import behaviortree.config;
export import behaviortree.basic_types;
export import behaviortree.blackboard;
export import behaviortree.tree_node;
export import behaviortree.leaf_node;
export import behaviortree.action_node;
export import behaviortree.condition_node;
export import behaviortree.control_node;
export import behaviortree.decorator_node;
export import behaviortree.factory;
export import behaviortree.json_export;
export import behaviortree.json_parsing;

// action
export import behaviortree.always_failure_node;
export import behaviortree.always_success_node;
export import behaviortree.pop_from_queue;
export import behaviortree.script_condition;
export import behaviortree.script_node;
export import behaviortree.set_blackboard_node;
export import behaviortree.sleep_node;
export import behaviortree.updated_action;
export import behaviortree.unset_blackboard_node;
export import behaviortree.test_node;

// control
export import behaviortree.fallback_node;
export import behaviortree.if_then_else_node;
export import behaviortree.manual_node;
export import behaviortree.parallel_all_node;
export import behaviortree.parallel_node;
export import behaviortree.reactive_fallback;
export import behaviortree.reactive_sequence;
export import behaviortree.sequence_node;
export import behaviortree.sequence_with_memory_node;
export import behaviortree.switch_node;
export import behaviortree.while_do_else_node;

// decorator
export import behaviortree.delay_node;
export import behaviortree.force_failure_node;
export import behaviortree.force_success_node;
export import behaviortree.inverter_node;
export import behaviortree.keep_running_until_failure_node;
export import behaviortree.loop_node;
export import behaviortree.repeat_node;
export import behaviortree.retry_node;
export import behaviortree.run_once_node;
export import behaviortree.script_precondition;
export import behaviortree.subtree_node;
export import behaviortree.timeout_node;
export import behaviortree.updated_decorator;

namespace behaviortree {
//Call the visitor for each node of the tree, given a root.
void ApplyRecursiveVisitor(const TreeNode *pTreeNode,const std::function<void(const TreeNode *)> &rVisitor);

//Call the visitor for each node of the tree, given a root.
void ApplyRecursiveVisitor(TreeNode *pTreeNode, const std::function<void(TreeNode *)> &rVisitor);

/**
 * Debug function to print the hierarchy of the tree. Prints to std::cout by default.
 */
void PrintTreeRecursively(const TreeNode *pRootNode, std::ostream &pNode = std::cout);

using SerializedTreeStatus = std::vector<std::pair<uint16_t, uint8_t>>;

/**
 * @brief buildSerializedStatusSnapshot can be used to create a buffer that can be stored
 * (or sent to a client application) to know the status of all the nodes of a tree.
 * It is not "human readable".
 *
 * @param root_node
 * @param serialized_buffer is the output.
 */
void BuildSerializedStatusSnapshot(const TreeNode *pRootNode, SerializedTreeStatus &rSerializedBuffer);

/// Simple way to extract the Type of a TreeNode at COMPILE TIME.
/// Useful to avoid the cost of dynamic_cast or the virtual method TreeNode::Type().
template<typename T>
inline NodeType GetType() {
    if(std::is_base_of<ActionNodeBase, T>::value)
        return NodeType::Action;
    if(std::is_base_of<ConditionNode, T>::value)
        return NodeType::Condition;
    if(std::is_base_of<SubtreeNode, T>::value)
        return NodeType::Subtree;
    if(std::is_base_of<DecoratorNode, T>::value)
        return NodeType::Decorator;
    if(std::is_base_of<ControlNode, T>::value)
        return NodeType::Control;
    return NodeType::Undefined;
}

int GetLibraryVersionNumber();

const char *GetLibraryVersionString();
}// namespace behaviortree

// module behaviortree;
// module;
