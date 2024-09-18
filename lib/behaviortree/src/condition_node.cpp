module behaviortree.condition_node;

namespace behaviortree {
ConditionNode::ConditionNode(const std::string &rName, const NodeConfig &rConfig): LeafNode::LeafNode(rName, rConfig) {}

SimpleConditionNode::SimpleConditionNode(const std::string &rName, TickFunctor tickFunctor, const NodeConfig &rConfig): ConditionNode(rName, rConfig), m_tickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleConditionNode::Tick() {
    return m_tickFunctor(*this);
}
}// namespace behaviortree

// module behaviortree.condition_node;
