#ifndef BEHAVIORTREE_SWITCH_NODE_H
#define BEHAVIORTREE_SWITCH_NODE_H

#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SwitchNode is equivalent to a switch statement, where a certain
 * branch (child) is executed according to the value of a blackboard entry.
 *
 * Note that the same behaviour can be achieved with multiple Sequences, Fallbacks and
 * Conditions reading the blackboard, but switch is shorter and more readable.
 *
 * Example usage:
 *

<Switch3 variable="{var}"  case_1="1" case_2="42" case_3="666" >
   <ActionA name="action_when_var_eq_1" />
   <ActionB name="action_when_var_eq_42" />
   <ActionC name="action_when_var_eq_666" />
   <ActionD name="default_action" />
 </Switch3>

When the SwitchNode is executed (Switch3 is a node with 3 cases)
the "variable" will be compared to the cases and execute the correct child
or the default one (last).
 *
 */

namespace details {

bool CheckStringEquality(const std::string& refV1, const std::string& refV2,
                         const ScriptingEnumsRegistry* ptrEnums);
}// namespace details

template<size_t NUM_CASES>
class SwitchNode: public ControlNode {
 public:
    SwitchNode(const std::string& refName, const behaviortree::NodeConfig& refConfig);

    virtual ~SwitchNode() override = default;

    void Halt() override;

    static PortsList ProvidedPorts();

 private:
    int32_t m_RunningChild;
    std::vector<std::string> m_CaseKeys;
    virtual behaviortree::NodeStatus Tick() override;
};

//-----------------------------------------------
//-----------------------------------------------

template<size_t NUM_CASES>
inline SwitchNode<NUM_CASES>::SwitchNode(const std::string& refName,
                                         const NodeConfig& refConfig)
    : ControlNode::ControlNode(refName, refConfig), m_RunningChild(-1) {
    SetRegistrationID("Switch");
    for(unsigned i = 1; i <= NUM_CASES; i++) {
        m_CaseKeys.push_back(std::string("case_") + std::to_string(i));
    }
}

template<size_t NUM_CASES>
inline void SwitchNode<NUM_CASES>::Halt() {
    m_RunningChild = -1;
    ControlNode::Halt();
}

template<size_t NUM_CASES>
inline PortsList SwitchNode<NUM_CASES>::ProvidedPorts() {
    static PortsList s_ProvidedPorts = []() {
        PortsList ports;
        ports.insert(behaviortree::InputPort<std::string>("variable"));
        for(uint32_t i = 1; i <= NUM_CASES; i++) {
            auto key = std::string("case_") + std::to_string(i);
            ports.insert(behaviortree::InputPort<std::string>(key));
        }
        return ports;
    }();

    return s_ProvidedPorts;
}

template<size_t NUM_CASES>
inline NodeStatus SwitchNode<NUM_CASES>::Tick() {
    if(ChildrenCount() != NUM_CASES + 1) {
        throw LogicError(
                "Wrong number of children in SwitchNode; "
                "must be (num_cases + default)");
    }

    std::string variable;
    std::string value;
    int32_t matchIndex = int32_t(NUM_CASES);// default index;

    // no variable? jump to default
    if(GetInput("variable", variable)) {
        // check each case until you find a match
        for(int32_t index = 0; index < int32_t(NUM_CASES); ++index) {
            const std::string& refCaseKey = m_CaseKeys[index];
            if(getInput(refCaseKey, value)) {
                if(details::CheckStringEquality(variable, value, this->Config().enums.get())) {
                    matchIndex = index;
                    break;
                }
            }
        }
    }

    // if another one was running earlier, halt it
    if(m_RunningChild != -1 && m_RunningChild != matchIndex) {
        haltChild(m_RunningChild);
    }

    auto& refSelectedChild = m_ChildrenNodes[matchIndex];
    NodeStatus ret = refSelectedChild->ExecuteTick();
    if(ret == NodeStatus::SKIPPED) {
        // if the matching child is SKIPPED, should I jump to default or
        // be SKIPPED myself? Going with the former, for the time being.
        m_RunningChild = -1;
        return NodeStatus::SKIPPED;
    } else if(ret == NodeStatus::RUNNING) {
        m_RunningChild = matchIndex;
    } else {
        ResetChildren();
        m_RunningChild = -1;
    }
    return ret;
}

}// namespace behaviortree

#endif// BEHAVIORTREE_SWITCH_NODE_H
