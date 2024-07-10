#define MINICORO_IMPL
#include "behaviortree/action_node.h"

#include "minicoro/minicoro.h"

using namespace behaviortree;

ActionNodeBase::ActionNodeBase(
        const std::string& refName, const NodeConfig& refConfig
): LeafNode::LeafNode(refName, refConfig) {}

//-------------------------------------------------------

SimpleActionNode::SimpleActionNode(
        const std::string& refName, SimpleActionNode::TickFunctor tickFunctor,
        const NodeConfig& refConfig
): SyncActionNode(refName, refConfig),
   m_TickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleActionNode::Tick() {
    NodeStatus prevStatus = NodeStatus();

    if(prevStatus == NodeStatus::IDLE) {
        SetNodeStatus(NodeStatus::RUNNING);
        prevStatus = NodeStatus::RUNNING;
    }

    NodeStatus status = m_TickFunctor(*this);
    if(status != prevStatus) {
        SetNodeStatus(status);
    }
    return status;
}

//-------------------------------------------------------

SyncActionNode::SyncActionNode(
        const std::string& name, const NodeConfig& config
): ActionNodeBase(name, config) {}

NodeStatus SyncActionNode::ExecuteTick() {
    auto nodeStatus = ActionNodeBase::ExecuteTick();
    if(nodeStatus == NodeStatus::RUNNING) {
        throw LogicError("SyncActionNode MUST never return RUNNING");
    }
    return nodeStatus;
}

//-------------------------------------

struct CoroActionNode::Pimpl {
    mco_coro* ptrCoro{nullptr};
    mco_desc desc;
};

void CoroEntry(mco_coro* co) {
    static_cast<CoroActionNode*>(co->user_data)->TickImpl();
}

CoroActionNode::CoroActionNode(
        const std::string& refName, const NodeConfig& refConfig
): ActionNodeBase(refName, refConfig),
   m_P(new Pimpl) {}

CoroActionNode::~CoroActionNode() {
    DestroyCoroutine();
}

void CoroActionNode::SetStatusRunningAndYield() {
    SetNodeStatus(NodeStatus::RUNNING);
    mco_yield(m_P->ptrCoro);
}

NodeStatus CoroActionNode::ExecuteTick() {
    // create a new coroutine, if necessary
    if(m_P->ptrCoro == nullptr) {
        // First initialize a `desc` object through `mco_desc_init`.
        m_P->desc = mco_desc_init(CoroEntry, 0);
        m_P->desc.user_data = this;

        mco_result res = mco_create(&m_P->ptrCoro, &m_P->desc);
        if(res != MCO_SUCCESS) {
            throw RuntimeError("Can't create coroutine");
        }
    }

    //------------------------
    // execute the coroutine
    mco_resume(m_P->ptrCoro);
    //------------------------

    // check if the coroutine finished. In this case, destroy it
    if(mco_status(m_P->ptrCoro) == MCO_DEAD) {
        DestroyCoroutine();
    }

    return GetNodeStatus();
}

void CoroActionNode::TickImpl() {
    SetNodeStatus(TreeNode::ExecuteTick());
}

void CoroActionNode::Halt() {
    DestroyCoroutine();
    ResetNodeStatus();// might be redundant
}

void CoroActionNode::DestroyCoroutine() {
    if(m_P->ptrCoro) {
        mco_result res = mco_destroy(m_P->ptrCoro);
        if(res != MCO_SUCCESS) {
            throw RuntimeError("Can't destroy coroutine");
        }
        m_P->ptrCoro = nullptr;
    }
}

bool StatefulActionNode::IsHaltRequested() const {
    return m_HaltRequested.load();
}

NodeStatus StatefulActionNode::Tick() {
    const NodeStatus preNodeStatus = GetNodeStatus();

    if(preNodeStatus == NodeStatus::IDLE) {
        NodeStatus newNodeStatus = OnStart();
        if(newNodeStatus == NodeStatus::IDLE) {
            throw LogicError(
                    "StatefulActionNode::onStart() must not return IDLE"
            );
        }
        return newNodeStatus;
    }
    //------------------------------------------
    if(preNodeStatus == NodeStatus::RUNNING) {
        NodeStatus newNodeStatus = OnRunning();
        if(newNodeStatus == NodeStatus::IDLE) {
            throw LogicError(
                    "StatefulActionNode::onRunning() must not return IDLE"
            );
        }
        return newNodeStatus;
    }
    return preNodeStatus;
}

void StatefulActionNode::Halt() {
    m_HaltRequested.store(true);
    if(GetNodeStatus() == NodeStatus::RUNNING) {
        OnHalted();
    }
    ResetNodeStatus();// might be redundant
}

NodeStatus behaviortree::ThreadedAction::ExecuteTick() {
    using LockType = std::unique_lock<std::mutex>;
    //send signal to other thread.
    // The other thread is in charge for changing the status
    if(GetNodeStatus() == NodeStatus::IDLE) {
        SetNodeStatus(NodeStatus::RUNNING);
        m_HaltRequested = false;
        m_ThreadHandle = std::async(std::launch::async, [this]() {
            try {
                auto nodeStatus = Tick();
                if(!IsHaltRequested()) {
                    SetNodeStatus(nodeStatus);
                }
            } catch(std::exception&) {
                std::cerr << "\nUncaught exception from tick(): ["
                          << GetRegistrAtionName() << "/" << GetNodeName()
                          << "]\n"
                          << std::endl;
                // Set the exception pointer and the status atomically.
                LockType lock(m_Mutex);
                m_Exptr = std::current_exception();
                SetNodeStatus(behaviortree::NodeStatus::IDLE);
            }
            EmitWakeUpSignal();
        });
    }

    LockType lock(m_Mutex);
    if(m_Exptr) {
        // The official interface of std::exception_ptr does not define any move
        // semantics. Thus, we copy and reset exptr_ manually.
        const auto exptrCopy = m_Exptr;
        m_Exptr = nullptr;
        std::rethrow_exception(exptrCopy);
    }
    return GetNodeStatus();
}

void ThreadedAction::Halt() {
    m_HaltRequested.store(true);

    if(m_ThreadHandle.valid()) {
        m_ThreadHandle.wait();
    }
    m_ThreadHandle = {};
    ResetNodeStatus();// might be redundant
}
