module;

export module behaviortree.switch_node;

#include "behaviortree/behaviortree_common.h"
#include "behaviortree/control_node.h"

namespace behaviortree {
/**
 * @brief The SwitchNode is equivalent to a switch statement, where a certain
 * branch (GetChildNode) is executed according to the value of a blackboard entry.
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
the "variable" will be compared to the cases and execute the correct GetChildNode
or the default one (last).
 *
 */

namespace details {
bool CheckStringEquality(const std::string &rStr, const std::string &rResult, const ScriptingEnumsRegistry *pEnums);
}// namespace details

template<size_t NUM_CASES>
class SwitchNode: public ControlNode {
 public:
    SwitchNode(const std::string &rName, const behaviortree::NodeConfig &rConfig);

    virtual ~SwitchNode() override = default;

    void Halt() override;

    static PortMap ProvidedPorts();

 private:
    int32_t m_RunningChild;
    std::vector<std::string> m_CaseKeyVec;
    virtual behaviortree::NodeStatus Tick() override;
};

//-----------------------------------------------
//-----------------------------------------------

template<size_t NUM_CASES>
inline SwitchNode<NUM_CASES>::SwitchNode(const std::string &rName, const NodeConfig &rConfig): ControlNode::ControlNode(rName, rConfig), m_RunningChild(-1) {
    SetRegistrationId("Switch");
    for(unsigned i = 1; i <= NUM_CASES; i++) {
        m_CaseKeyVec.push_back(std::string("case_") + std::to_string(i));
    }
}

template<size_t NUM_CASES>
inline void SwitchNode<NUM_CASES>::Halt() {
    m_RunningChild = -1;
    ControlNode::Halt();
}

template<size_t NUM_CASES>
inline PortMap SwitchNode<NUM_CASES>::ProvidedPorts() {
    static PortMap s_ProvidedPorts = []() {
        PortMap portMap;
        portMap.insert(behaviortree::InputPort<std::string>("variable"));
        for(uint32_t i = 1; i <= NUM_CASES; i++) {
            auto key = std::string("case_") + std::to_string(i);
            portMap.insert(behaviortree::InputPort<std::string>(key));
        }
        return portMap;
    }();

    return s_ProvidedPorts;
}

template<size_t NUM_CASES>
inline NodeStatus SwitchNode<NUM_CASES>::Tick() {
    if(GetChildrenNum() != NUM_CASES + 1) {
        throw util::LogicError(
                "Wrong number of GetChildrenNode in SwitchNode; "
                "must be (num_cases + default)"
        );
    }

    std::string variable;
    std::string value;
    int32_t matchIndex = int32_t(NUM_CASES);// default index;

    // no variable? jump to default
    if(GetInput("variable", variable)) {
        // check each case until you find a match
        for(int32_t index = 0; index < int32_t(NUM_CASES); ++index) {
            const std::string &rCaseKey = m_CaseKeyVec[index];
            if(GetInput(rCaseKey, value)) {
                if(details::CheckStringEquality(variable, value, this->GetConfig().pEnums.get())) {
                    matchIndex = index;
                    break;
                }
            }
        }
    }

    // if another one was running earlier, halt it
    if(m_RunningChild != -1 && m_RunningChild != matchIndex) {
        HaltChild(m_RunningChild);
    }

    auto &rSelectedChild = m_childrenNodeVec[matchIndex];
    NodeStatus ret = rSelectedChild->ExecuteTick();
    if(ret == NodeStatus::Skipped) {
        // if the matching child is SKIPPED, should I jump to default or
        // be SKIPPED myself? Going with the former, for the time being.
        m_RunningChild = -1;
        return NodeStatus::Skipped;
    } else if(ret == NodeStatus::Running) {
        m_RunningChild = matchIndex;
    } else {
        ResetChildren();
        m_RunningChild = -1;
    }
    return ret;
}

}// namespace behaviortree

// module behaviortree.switch_node;
// module;
