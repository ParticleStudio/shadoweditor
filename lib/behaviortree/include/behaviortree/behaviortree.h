#ifndef BEHAVIORTREE_BEHAVIORTREE_H
#define BEHAVIORTREE_BEHAVIORTREE_H

#include <iostream>

#include "behaviortree/action/always_failure_node.hpp"
#include "behaviortree/action/always_success_node.hpp"
#include "behaviortree/action/script_condition.hpp"
#include "behaviortree/action/script_node.hpp"
#include "behaviortree/action/set_blackboard_node.hpp"
#include "behaviortree/action/sleep_node.hpp"
#include "behaviortree/action/test_node.hpp"
#include "behaviortree/action/unset_blackboard_node.hpp"
#include "behaviortree/action/updated_action.hpp"
#include "behaviortree/action_node.h"
#include "behaviortree/common.h"
#include "behaviortree/condition_node.h"
#include "behaviortree/control/fallback_node.hpp"
#include "behaviortree/control/if_then_else_node.hpp"
#include "behaviortree/control/parallel_all_node.hpp"
#include "behaviortree/control/parallel_node.hpp"
#include "behaviortree/control/reactive_fallback.hpp"
#include "behaviortree/control/reactive_sequence.hpp"
#include "behaviortree/control/sequence_node.hpp"
#include "behaviortree/control/sequence_with_memory_node.hpp"
#include "behaviortree/control/switch_node.hpp"
#include "behaviortree/control/while_do_else_node.hpp"
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
void ApplyRecursiveVisitor(const TreeNode *pTreeNode, const std::function<void(const TreeNode *)> &rVisitor);

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
}// namespace behaviortree

#endif// BEHAVIORTREE_BEHAVIORTREE_H
