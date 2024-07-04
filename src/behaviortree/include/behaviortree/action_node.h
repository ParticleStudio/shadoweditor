#ifndef BEHAVIORTREE_ACTION_NODE_H
#define BEHAVIORTREE_ACTION_NODE_H

#include <atomic>
#include <future>
#include <mutex>
#include <thread>

#include "leaf_node.h"

namespace behaviortree {
// IMPORTANT: Actions which returned SUCCESS or FAILURE will not be ticked
// again unless resetStatus() is called first.
// Keep this in mind when writing your custom Control and Decorator nodes.

/**
 * @brief The ActionNodeBase is the base class to use to create any kind of action.
 * A particular derived class is free to override executeTick() as needed.
 *
 */
class ActionNodeBase: public LeafNode {
 public:
    ActionNodeBase(const std::string &refName, const NodeConfig &refConfig);
    ~ActionNodeBase() override = default;

    virtual NodeType Type() const override final {
        return NodeType::ACTION;
    }
};

/**
 * @brief The SyncActionNode is an ActionNode that
 * explicitly prevents the status RUNNING and doesn't require
 * an implementation of halt().
 */
class SyncActionNode: public ActionNodeBase {
 public:
    SyncActionNode(const std::string &refName, const NodeConfig &refConfig);
    ~SyncActionNode() override = default;

    /// throws if the derived class return RUNNING.
    virtual NodeStatus ExecuteTick() override;

    /// You don't need to override this
    virtual void Halt() override final {
        ResetNodeStatus();
    }
};

/**
 * @brief The SimpleActionNode provides an easy to use SyncActionNode.
 * The user should simply provide a callback with this signature
 *
 *    BT::NodeStatus functionName(TreeNode&)
 *
 * This avoids the hassle of inheriting from a ActionNode.
 *
 * Using lambdas or std::bind it is easy to pass a pointer to a method.
 * SimpleActionNode is executed synchronously and does not support halting.
 */
class SimpleActionNode: public SyncActionNode {
 public:
    using TickFunctor = std::function<NodeStatus(TreeNode &)>;

    // You must provide the function to call when tick() is invoked
    SimpleActionNode(const std::string &refName, TickFunctor tickFunctor,
                     const NodeConfig &refConfig);

    ~SimpleActionNode() override = default;

 protected:
    virtual NodeStatus Tick() override final;

    TickFunctor m_TickFunctor;
};

/**
 * @brief The ThreadedAction executes the tick in a different thread.
 *
 * IMPORTANT: this action is quite hard to implement correctly.
 * Please make sure that you know what you are doing.
 *
 * - In your overriden tick() method, you must check periodically
 *   the result of the method IsHaltRequested() and stop your execution accordingly.
 *
 * - in the overriden halt() method, you can do some cleanup, but do not forget to
 *   invoke the base class method ThreadedAction::halt();
 *
 * - remember, with few exceptions, a halted ThreadedAction must return NodeStatus::IDLE.
 *
 * For a complete example, look at __AsyncActionTest__ in action_test_node.h in the folder test.
 *
 * NOTE: when the thread is completed, i.e. the tick() returns its status,
 * a TreeNode::emitWakeUpSignal() will be called.
 */

class ThreadedAction: public ActionNodeBase {
 public:
    ThreadedAction(const std::string &refName, const NodeConfig &refConfig)
        : ActionNodeBase(refName, refConfig) {}

    bool IsHaltRequested() const {
        return m_HaltRequested.load();
    }

    // This method spawn a new thread. Do NOT remove the "final" keyword.
    virtual NodeStatus ExecuteTick() override final;

    virtual void Halt() override;

 private:
    std::exception_ptr m_Exptr;
    std::atomic_bool m_HaltRequested{false};
    std::future<void> m_ThreadHandle;
    std::mutex m_Mutex;
};

/**
 * @brief The StatefulActionNode is the preferred way to implement asynchronous Actions.
 * It is actually easier to use correctly, when compared with ThreadedAction
 *
 * It is particularly useful when your code contains a request-reply pattern,
 * i.e. when the actions sends an asynchronous request, then checks periodically
 * if the reply has been received and, eventually, analyze the reply to determine
 * if the result is SUCCESS or FAILURE.
 *
 * -) an action that was in IDLE state will call onStart()
 *
 * -) A RUNNING action will call onRunning()
 *
 * -) if halted, method onHalted() is invoked
 */
class StatefulActionNode: public ActionNodeBase {
 public:
    StatefulActionNode(const std::string &refName, const NodeConfig &refConfig)
        : ActionNodeBase(refName, refConfig) {}

    /// Method called once, when transitioning from the state IDLE.
    /// If it returns RUNNING, this becomes an asynchronous node.
    virtual NodeStatus OnStart() = 0;

    /// method invoked when the action is already in the RUNNING state.
    virtual NodeStatus OnRunning() = 0;

    /// when the method halt() is called and the action is RUNNING, this method is invoked.
    /// This is a convenient place todo a cleanup, if needed.
    virtual void OnHalted() = 0;

    bool IsHaltRequested() const;

 protected:
    // do not override this method
    NodeStatus Tick() override final;
    // do not override this method
    void Halt() override final;

 private:
    std::atomic_bool m_HaltRequested{false};
};

/**
 * @brief The CoroActionNode class is an a good candidate for asynchronous actions
 * which need to communicate with an external service using an async request/reply interface.
 *
 * It is up to the user to decide when to suspend execution of the Action and resume
 * the parent node, invoking the method setStatusRunningAndYield().
 */
class CoroActionNode: public ActionNodeBase {
 public:
    CoroActionNode(const std::string &refName, const NodeConfig &refConfig);
    virtual ~CoroActionNode() override;

    /// Use this method to return RUNNING and temporary "pause" the Action.
    void SetStatusRunningAndYield();

    // This method triggers the TickEngine. Do NOT remove the "final" keyword.
    virtual NodeStatus ExecuteTick() override final;

    // Used internally, but it needs to be public
    void TickImpl();

    /** You may want to override this method. But still, remember to call this
    * implementation too.
    *
    * Example:
    *
    *     void MyAction::halt()
    *     {
    *         // do your stuff here
    *         CoroActionNode::halt();
    *     }
    */
    void Halt() override;

 protected:
    struct Pimpl;// The Pimpl idiom
    std::unique_ptr<Pimpl> m_P;

    void DestroyCoroutine();
};

}// namespace behaviortree

#endif// BEHAVIORTREE_ACTION_NODE_H
