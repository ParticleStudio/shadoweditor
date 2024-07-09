#include "behaviortree/condition_node.h"

namespace behaviortree {
ConditionNode::ConditionNode(const std::string& refName, const NodeConfig& refConfig)
    : LeafNode::LeafNode(refName, refConfig) {}

SimpleConditionNode::SimpleConditionNode(const std::string& refName,
                                         TickFunctor tickFunctor,
                                         const NodeConfig& refConfig)
    : ConditionNode(refName, refConfig), m_TickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleConditionNode::Tick() {
    return m_TickFunctor(*this);
}
}// namespace behaviortree
