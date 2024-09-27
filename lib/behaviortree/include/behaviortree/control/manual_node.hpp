//#ifndef BEHAVIORTREE_MANUAL_NODE_H
//#define BEHAVIORTREE_MANUAL_NODE_H
//
//#include "behaviortree/control_node.h"
//
//namespace behaviortree {
///**
// * @brief Use a Terminal User Interface (ncurses) to select a certain child manually.
// */
//class ManualSelectorNode: public ControlNode {
// public:
//    ManualSelectorNode(const std::string& refName, const NodeConfig& refConfig);
//
//    virtual ~ManualSelectorNode() override = default;
//
//    virtual void Halt() override;
//
//    static PortsList ProvidedPorts() {
//        return {InputPort<bool>(REPEAT_LAST_SELECTION, false,
//                                "If true, execute again the same child that was selected "
//                                "the "
//                                "last "
//                                "time")};
//    }
//
// private:
//    static constexpr const char* REPEAT_LAST_SELECTION = "repeat_last_selection";
//
//    virtual behaviortree::NodeStatus Tick() override;
//    int m_RunningChildIdx;
//    int m_PreviouslyExecutedIdx;
//
//    enum NumericalStatus {
//        NUM_SUCCESS = 253,
//        NUM_FAILURE = 254,
//        NUM_RUNNING = 255,
//    };
//
//    NodeStatus SelectNodeStatus() const;
//
//    uint8_t SelectChild() const;
//};
//
//}// namespace behaviortree
//
//#endif// BEHAVIORTREE_MANUAL_NODE_H
