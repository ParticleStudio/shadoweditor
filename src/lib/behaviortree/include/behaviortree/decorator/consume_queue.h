#ifndef BEHAVIORTREE_CONSUME_QUEUE_H
#define BEHAVIORTREE_CONSUME_QUEUE_H

#include <list>

#include "behaviortree/action/pop_from_queue.hpp"
#include "behaviortree/control_node.h"
#include "behaviortree/decorator_node.h"

namespace behaviortree {
/**
 * Execute the child node as long as the queue is not empty.
 * At each iteration, an item of type T is popped from the "queue" and
 * inserted in "popped_item".
 *
 * An empty queue will return SUCCESS
 */

template<typename T>
class [[deprecated("You are encouraged to use the LoopNode instead"
)]] ConsumeQueue: public DecoratorNode {
 public:
    ConsumeQueue(const std::string& refName, const NodeConfig& refConfig): DecoratorNode(refName, refConfig) {}

    NodeStatus Tick() override {
        // by default, return SUCCESS, even if queue is empty
        NodeStatus statusToBeReturned = NodeStatus::SUCCESS;

        if(m_RunningChild) {
            NodeStatus childState = m_ChildNode->ExecuteTick();
            m_RunningChild = (childState == NodeStatus::RUNNING);
            if(m_RunningChild) {
                return NodeStatus::RUNNING;
            } else {
                HaltChild();
                statusToBeReturned = childState;
            }
        }

        std::shared_ptr<ProtectedQueue<T>> ptrQueue;
        if(getInput("queue", ptrQueue) && ptrQueue) {
            std::unique_lock<std::mutex> lk(ptrQueue->mtx);
            auto& items = ptrQueue->items;

            while(!items.empty()) {
                SetNodeStatus(NodeStatus::RUNNING);

                T val = items.front();
                items.pop_front();
                setOutput("popped_item", val);

                lk.unlock();
                NodeStatus childState = m_ChildNode->ExecuteTick();
                lk.lock();

                m_RunningChild = (childState == NodeStatus::RUNNING);
                if(m_RunningChild) {
                    return NodeStatus::RUNNING;
                } else {
                    HaltChild();
                    if(childState == NodeStatus::FAILURE) {
                        return NodeStatus::FAILURE;
                    }
                    statusToBeReturned = childState;
                }
            }
        }

        return statusToBeReturned;
    }

    static PortsList ProvidedPorts() {
        return {InputPort<std::shared_ptr<ProtectedQueue<T>>>("queue"),
                OutputPort<T>("poppe"
                              "d_"
                              "ite"
                              "m")};
    }

 private:
    bool m_RunningChild{false};
};

}// namespace behaviortree

#endif// BEHAVIORTREE_CONSUME_QUEUE_H
