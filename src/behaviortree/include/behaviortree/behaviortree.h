#ifndef BEHAVIORTREE_BEHAVIORTREE_H
#define BEHAVIORTREE_BEHAVIORTREE_H

#include <iostream>

#include "behaviortree/action/always_failure_node.h"
#include "behaviortree/action/always_success_node.h"
#include "behaviortree/action/script_condition.h"
#include "behaviortree/action/script_node.h"
#include "behaviortree/action/set_blackboard_node.h"
#include "behaviortree/action/sleep_node.h"
#include "behaviortree/action/test_node.h"
#include "behaviortree/action/unset_blackboard_node.h"
#include "behaviortree/action/updated_action.h"
#include "behaviortree/action_node.h"
#include "behaviortree/condition_node.h"
#include "behaviortree/control/fallback_node.h"
#include "behaviortree/control/if_then_else_node.h"
#include "behaviortree/control/parallel_all_node.h"
#include "behaviortree/control/parallel_node.h"
#include "behaviortree/control/reactive_fallback.h"
#include "behaviortree/control/reactive_sequence.h"
#include "behaviortree/control/sequence_node.h"
#include "behaviortree/control/sequence_with_memory_node.h"
#include "behaviortree/control/switch_node.h"
#include "behaviortree/control/while_do_else_node.h"
#include "behaviortree/decorator/delay_node.h"
#include "behaviortree/decorator/force_failure_node.h"
#include "behaviortree/decorator/force_success_node.h"
#include "behaviortree/decorator/inverter_node.h"
#include "behaviortree/decorator/keep_running_until_failure_node.h"
#include "behaviortree/decorator/loop_node.h"
#include "behaviortree/decorator/repeat_node.h"
#include "behaviortree/decorator/retry_node.h"
#include "behaviortree/decorator/run_once_node.h"
#include "behaviortree/decorator/script_precondition.h"
#include "behaviortree/decorator/subtree_node.h"
#include "behaviortree/decorator/timeout_node.h"
#include "behaviortree/decorator/updated_decorator.h"

namespace behaviortree {
//Call the visitor for each node of the tree, given a root.
void ApplyRecursiveVisitor(const TreeNode* ptrRootNode,
                           const std::function<void(const TreeNode*)>& refVisitor);

//Call the visitor for each node of the tree, given a root.
void ApplyRecursiveVisitor(TreeNode* ptrRootNode,
                           const std::function<void(TreeNode*)>& refVisitor);

/**
 * Debug function to print the hierarchy of the tree. Prints to std::cout by default.
 */
void PrintTreeRecursively(const TreeNode* ptrRootNode, std::ostream& refStream = std::cout);

using SerializedTreeStatus = std::vector<std::pair<uint16_t, uint8_t>>;

/**
 * @brief buildSerializedStatusSnapshot can be used to create a buffer that can be stored
 * (or sent to a client application) to know the status of all the nodes of a tree.
 * It is not "human readable".
 *
 * @param root_node
 * @param serialized_buffer is the output.
 */
void BuildSerializedStatusSnapshot(const TreeNode* ptrRootNode,
                                   SerializedTreeStatus& refSerializedBuffer);

/// Simple way to extract the type of a TreeNode at COMPILE TIME.
/// Useful to avoid the cost of dynamic_cast or the virtual method TreeNode::type().
template<typename T>
inline NodeType GetType() {
    if(std::is_base_of<ActionNodeBase, T>::value) return NodeType::ACTION;
    if(std::is_base_of<ConditionNode, T>::value) return NodeType::CONDITION;
    if(std::is_base_of<SubTreeNode, T>::value) return NodeType::SUBTREE;
    if(std::is_base_of<DecoratorNode, T>::value) return NodeType::DECORATOR;
    if(std::is_base_of<ControlNode, T>::value) return NodeType::CONTROL;
    return NodeType::UNDEFINED;
}

const char* LibraryVersionString();

int LibraryVersionNumber();

}// namespace behaviortree

#endif// BEHAVIORTREE_BEHAVIORTREE_H
