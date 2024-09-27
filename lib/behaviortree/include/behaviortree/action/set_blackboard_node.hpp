#ifndef BEHAVIORTREE_SET_BLACKBOARD_NODE_HPP
#define BEHAVIORTREE_SET_BLACKBOARD_NODE_HPP

import common.exception;

#include "behaviortree/action_node.h"
#include "behaviortree/common.h"

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
class BEHAVIORTREE_API SetBlackboardNode: public SyncActionNode {
 public:
    SetBlackboardNode(const std::string &rName, const NodeConfig &rConfig): SyncActionNode(rName, rConfig) {
        SetRegistrationId("SetBlackboard");
    }

    static PortMap ProvidedPorts() {
        return {InputPort("value", "Value to be written int othe output_key"), BidirectionalPort("output_key", "GetNodeName of the blackboard entry where the value should be written")};
    }

 private:
    virtual behaviortree::NodeStatus Tick() override {
        std::string outputKey;
        if(!GetInput("outputKey", outputKey)) {
            throw util::RuntimeError("missing port [outputKey]");
        }

        const std::string valueStr = GetConfig().inputPortMap.at("value");

        std::string_view strippedKey;
        if(IsBlackboardPointer(valueStr, &strippedKey)) {
            const auto inputKey = std::string(strippedKey);
            std::shared_ptr<Blackboard::Entry> pSrcEntry = GetConfig().pBlackboard->GetEntry(inputKey);
            std::shared_ptr<Blackboard::Entry> pDstEntry = GetConfig().pBlackboard->GetEntry(outputKey);

            if(!pSrcEntry) {
                throw util::RuntimeError("Can't find the port referred by [value]");
            }
            if(!pDstEntry) {
                GetConfig().pBlackboard->CreateEntry(outputKey, pSrcEntry->typeInfo);
                pDstEntry = GetConfig().pBlackboard->GetEntry(outputKey);
            }
            pDstEntry->value = pSrcEntry->value;
        } else {
            GetConfig().pBlackboard->Set(outputKey, valueStr);
        }

        return NodeStatus::Success;
    }
};
}// namespace behaviortree

#endif// BEHAVIORTREE_SET_BLACKBOARD_NODE_HPP
