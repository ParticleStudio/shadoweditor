#ifndef BEHAVIORTREE_LOOP_NODE_H
#define BEHAVIORTREE_LOOP_NODE_H

#include <deque>

#include "behaviortree/decorator_node.h"

namespace behaviortree {

// this object will allow us to modify the queue in place,
// when popping, in a thread safe-way and without copying the entire queue.
template<typename T>
using SharedQueue = std::shared_ptr<std::deque<T>>;

/**
 * @brief The LoopNode class is used to pop_front elements from a std::deque.
 * This element is copied into the port "value" and the GetChild will be executed,
 * as long as we have elements in the queue.
 *
 * See Example 4: ex04_waypoints
 *
 * NOTE: unless T is `Any`, `string` or `double`, you must register the loop manually into
 * the factory.
 */
template<typename T = Any>
class LoopNode: public DecoratorNode {
    bool m_ChildRunning{false};
    SharedQueue<T> m_StaticQueue;
    SharedQueue<T> m_CurrentQueue;

 public:
    LoopNode(const std::string& refName, const NodeConfig& refConfig): DecoratorNode(refName, refConfig) {
        auto rawPort = GetRawPortValue("queue");
        if(!IsBlackboardPointer(rawPort)) {
            m_StaticQueue = ConvertFromString<SharedQueue<T>>(rawPort);
        }
    }

    NodeStatus Tick() override {
        bool popped{false};
        if(GetNodeStatus() == NodeStatus::IDLE) {
            m_ChildRunning = false;
            // special case: the port contains a string that was converted to SharedQueue<T>
            if(m_StaticQueue) {
                m_CurrentQueue = m_StaticQueue;
            }
        }

        // Pop value from queue, if the child is not RUNNING
        if(!m_ChildRunning) {
            // if the port is static, any_ref is empty, otherwise it will keep access to
            // port locked for thread-safety
            AnyPtrLocked anyRef = m_StaticQueue ? AnyPtrLocked()
                                                : GetLockedPortContent("queue");
            if(anyRef) {
                m_CurrentQueue = anyRef.Get()->Cast<SharedQueue<T>>();
            }

            if(m_CurrentQueue && !m_CurrentQueue->empty()) {
                auto value = std::move(m_CurrentQueue->front());
                m_CurrentQueue->pop_front();
                popped = true;
                SetOutput("value", value);
            }
        }

        if(!popped && !m_ChildRunning) {
            return GetInput<NodeStatus>("if_empty").value();
        }

        if(GetNodeStatus() == NodeStatus::IDLE) {
            SetNodeStatus(NodeStatus::RUNNING);
        }

        NodeStatus childState = m_ChildNode->ExecuteTick();
        m_ChildRunning = (childState == NodeStatus::RUNNING);

        if(IsStatusCompleted(childState)) {
            ResetChild();
        }

        if(childState == NodeStatus::FAILURE) {
            return NodeStatus::FAILURE;
        }
        return NodeStatus::RUNNING;
    }

    static PortsList ProvidedPorts() {
        // we mark "queue" as BidirectionalPort, because the original element is modified
        return {BidirectionalPort<SharedQueue<T>>("queue"),
                InputPort<NodeStatus>(
                        "if_empty", NodeStatus::SUCCESS,
                        "NodeStatus to return if queue is Empty: "
                        "SUCCESS, FAILURE, SKIPPED"
                ),
                OutputPort<T>("value")};
    }
};

template<>
inline SharedQueue<int> ConvertFromString<SharedQueue<int>>(StringView str) {
    auto parts = SplitString(str, ';');
    SharedQueue<int> output = std::make_shared<std::deque<int>>();
    for(const StringView& refPart: parts) {
        output->push_back(ConvertFromString<int>(refPart));
    }
    return output;
}

template<>
inline SharedQueue<bool> ConvertFromString<SharedQueue<bool>>(StringView str) {
    auto parts = SplitString(str, ';');
    SharedQueue<bool> output = std::make_shared<std::deque<bool>>();
    for(const StringView& refPart: parts) {
        output->push_back(ConvertFromString<bool>(refPart));
    }
    return output;
}

template<>
inline SharedQueue<double> ConvertFromString<SharedQueue<double>>(StringView str
) {
    auto parts = SplitString(str, ';');
    SharedQueue<double> output = std::make_shared<std::deque<double>>();
    for(const StringView& refPart: parts) {
        output->push_back(ConvertFromString<double>(refPart));
    }
    return output;
}

template<>
inline SharedQueue<std::string> ConvertFromString<SharedQueue<std::string>>(
        StringView str
) {
    auto parts = SplitString(str, ';');
    SharedQueue<std::string> output =
            std::make_shared<std::deque<std::string>>();
    for(const StringView& refPart: parts) {
        output->push_back(ConvertFromString<std::string>(refPart));
    }
    return output;
}

}// namespace behaviortree

#endif// BEHAVIORTREE_LOOP_NODE_H
