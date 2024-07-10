#ifndef BEHAVIORTREE_POP_FROM_QUEUE_HPP
#define BEHAVIORTREE_POP_FROM_QUEUE_HPP

#include <list>
#include <mutex>

#include "behaviortree/action_node.h"
#include "behaviortree/decorator_node.h"

/**
 * Template Action used in ex04_waypoints.cpp example.
 *
 * Its purpose is to do make it easy to create while loops wich consume the elements of a queue.
 *
 * Note that modifying the queue is not thread safe, therefore the action that creates the queue
 * or push elements into it, must be Synchronous.
 *
 * When ticked, we pop_front from the "queue" and insert that value in "popped_item".
 * Return FAILURE if the queue is empty, SUCCESS otherwise.
 */
namespace behaviortree {

template<typename T>
struct ProtectedQueue {
    std::list<T> items;
    std::mutex mtx;
};

/*
 * Few words about why we represent the queue as std::shared_ptr<ProtectedQueue>:
 *
 * Since we will pop from the queue, the fact that the blackboard uses
 * a value semantic is not very convenient, since it would oblige us to
 * copy the entire std::list from the BB and than copy again a new one with one less element.
 *
 * We avoid this using reference semantic (wrapping the object in a shared_ptr).
 * Unfortunately, remember that this makes our access to the list not thread-safe!
 * This is the reason why we add a mutex to be used when modyfying the ProtectedQueue::items
 *
 * */

template<typename T>
class PopFromQueue: public SyncActionNode {
 public:
    PopFromQueue(const std::string &name, const NodeConfig &config): SyncActionNode(name, config) {}

    NodeStatus Tick() override {
        std::shared_ptr<ProtectedQueue<T>> queue;
        if(GetInput("queue", queue) && queue) {
            std::unique_lock<std::mutex> lk(queue->mtx);
            auto &items = queue->items;

            if(items.empty()) {
                return NodeStatus::FAILURE;
            } else {
                T val = items.front();
                items.pop_front();
                SetOutput("popped_item", val);
                return NodeStatus::SUCCESS;
            }
        } else {
            return NodeStatus::FAILURE;
        }
    }

    static PortsList ProvidedPorts() {
        return {InputPort<std::shared_ptr<ProtectedQueue<T>>>("queue"),
                OutputPort<T>("poppe"
                              "d_"
                              "ite"
                              "m")};
    }
};

/**
 * Get the size of a queue. Usefull is you want to write something like:
 *
 *  <QueueSize queue="{waypoints}" size="{wp_size}" />
 *  <Repeat num_cycles="{wp_size}" >
 *      <Sequence>
 *          <PopFromQueue  queue="{waypoints}" popped_item="{wp}" >
 *          <UseWaypoint   waypoint="{wp}" />
 *      </Sequence>
 *  </Repeat>
 */
template<typename T>
class QueueSize: public SyncActionNode {
 public:
    QueueSize(const std::string &name, const NodeConfig &config): SyncActionNode(name, config) {}

    NodeStatus Tick() override {
        std::shared_ptr<ProtectedQueue<T>> queue;
        if(GetInput("queue", queue) && queue) {
            std::unique_lock<std::mutex> lk(queue->mtx);
            auto &items = queue->items;

            if(items.empty()) {
                return NodeStatus::FAILURE;
            } else {
                SetOutput("size", int(items.size()));
                return NodeStatus::SUCCESS;
            }
        }
        return NodeStatus::FAILURE;
    }

    static PortsList ProvidedPorts() {
        return {InputPort<std::shared_ptr<ProtectedQueue<T>>>("queue"),
                OutputPort<int>("size")};
    }
};

}// namespace behaviortree

#endif// BEHAVIORTREE_POP_FROM_QUEUE_HPP
