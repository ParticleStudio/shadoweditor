#ifndef BEHAVIORTREE_SET_BLACKBOARD_NODE_H
#define BEHAVIORTREE_SET_BLACKBOARD_NODE_H

#include "behaviortree/action_node.h"

namespace behaviortree {
/**
 * @brief The SetBlackboard is action used to store a string
 * into an entry of the Blackboard specified in "output_key".
 *
 * Example usage:
 *
 *  <SetBlackboard value="42" output_key="the_answer" />
 *
 * Will store the string "42" in the entry with key "the_answer".
 *
 * Alternatively, you can use it to copy one port inside another port:
 *
 * <SetBlackboard value="{src_port}" output_key="dst_port" />
 *
 * This will copy the Type and content of {src_port} into {dst_port}
 */
class SetBlackboardNode: public SyncActionNode {
 public:
    SetBlackboardNode(const std::string& refName, const NodeConfig& refConfig)
        : SyncActionNode(refName, refConfig) {
        SetRegistrationID("SetBlackboard");
    }

    static PortsList ProvidedPorts() {
        return {InputPort("value", "Value to be written int othe output_key"),
                BidirectionalPort("output_key",
                                  "GetNodeName of the blackboard entry where the "
                                  "value should be written")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        std::string output_key;
        if(!GetInput("output_key", output_key)) {
            throw RuntimeError("missing port [output_key]");
        }

        const std::string valueStr = GetConfig().inputPortsMap.at("value");

        StringView strippedKey;
        if(IsBlackboardPointer(valueStr, &strippedKey)) {
            const auto input_key = std::string(strippedKey);
            std::shared_ptr<Blackboard::Entry> ptrSrcEntry =
                    GetConfig().ptrBlackboard->GetEntry(input_key);
            std::shared_ptr<Blackboard::Entry> ptrDstEntry =
                    GetConfig().ptrBlackboard->GetEntry(output_key);

            if(!ptrSrcEntry) {
                throw RuntimeError("Can't find the port referred by [value]");
            }
            if(!ptrDstEntry) {
                GetConfig().ptrBlackboard->CreateEntry(output_key, ptrSrcEntry->typeInfo);
                ptrDstEntry = GetConfig().ptrBlackboard->GetEntry(output_key);
            }
            ptrDstEntry->value = ptrSrcEntry->value;
        } else {
            GetConfig().ptrBlackboard->Set(output_key, valueStr);
        }

        return NodeStatus::SUCCESS;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_SET_BLACKBOARD_NODE_H
