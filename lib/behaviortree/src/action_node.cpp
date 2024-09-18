module behaviortree.action_node;

import common.util;

#define MINICORO_IMPL
#include "minicoro/minicoro.h"

namespace behaviortree {
ActionNodeBase::ActionNodeBase(const std::string &rName, const NodeConfig &rConfig): LeafNode::LeafNode(rName, rConfig) {}

//-------------------------------------------------------

SimpleActionNode::SimpleActionNode(const std::string &rName, SimpleActionNode::TickFunctor tickFunctor, const NodeConfig &rConfig): SyncActionNode(rName, rConfig), m_tickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleActionNode::Tick() {
    NodeStatus preNodeStatus = NodeStatus();

    if(preNodeStatus == NodeStatus::Idle) {
        SetNodeStatus(NodeStatus::Running);
        preNodeStatus = NodeStatus::Running;
    }

    NodeStatus status = m_tickFunctor(*this);
    if(status != preNodeStatus) {
        SetNodeStatus(status);
    }
    return status;
}

//-------------------------------------------------------

SyncActionNode::SyncActionNode(const std::string &rName, const NodeConfig &rConfig): ActionNodeBase(rName, rConfig) {}

NodeStatus SyncActionNode::ExecuteTick() {
    auto nodeStatus = ActionNodeBase::ExecuteTick();
    if(nodeStatus == NodeStatus::Running) {
        throw util::LogicError("SyncActionNode MUST never return RUNNING");
    }
    return nodeStatus;
}

//-------------------------------------

struct CoroActionNode::Pimpl {
    mco_coro *pCoro{nullptr};
    mco_desc desc;
};

void CoroEntry(mco_coro *pCoro) {
    static_cast<CoroActionNode *>(pCoro->user_data)->TickImpl();
}

CoroActionNode::CoroActionNode(const std::string &rName, const NodeConfig &rConfig): ActionNodeBase(rName, rConfig), m_pPimpl(new Pimpl) {}

CoroActionNode::~CoroActionNode() {
    DestroyCoroutine();
}

void CoroActionNode::SetStatusRunningAndYield() {
    SetNodeStatus(NodeStatus::Running);
    mco_yield(m_pPimpl->pCoro);
}

NodeStatus CoroActionNode::ExecuteTick() {
    // create a new coroutine, if necessary
    if(m_pPimpl->pCoro == nullptr) {
        // First initialize a `desc` object through `mco_desc_init`.
        m_pPimpl->desc = mco_desc_init(CoroEntry, 0);
        m_pPimpl->desc.user_data = this;

        mco_result res = mco_create(&m_pPimpl->pCoro, &m_pPimpl->desc);
        if(res != MCO_SUCCESS) {
            throw util::RuntimeError("Can't create coroutine");
        }
    }

    //------------------------
    // execute the coroutine
    mco_resume(m_pPimpl->pCoro);
    //------------------------

    // check if the coroutine finished. In this case, destroy it
    if(mco_status(m_pPimpl->pCoro) == MCO_DEAD) {
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
    if(m_pPimpl->pCoro) {
        mco_result res = mco_destroy(m_pPimpl->pCoro);
        if(res != MCO_SUCCESS) {
            throw util::RuntimeError("Can't destroy coroutine");
        }
        m_pPimpl->pCoro = nullptr;
    }
}

bool StatefulActionNode::IsHaltRequested() const {
    return m_haltRequested.load();
}

NodeStatus StatefulActionNode::Tick() {
    const NodeStatus preNodeStatus = GetNodeStatus();

    if(preNodeStatus == NodeStatus::Idle) {
        NodeStatus newNodeStatus = OnStart();
        if(newNodeStatus == NodeStatus::Idle) {
            throw util::LogicError(
                    "StatefulActionNode::onStart() must not return IDLE"
            );
        }
        return newNodeStatus;
    }
    //------------------------------------------
    if(preNodeStatus == NodeStatus::Running) {
        NodeStatus newNodeStatus = OnRunning();
        if(newNodeStatus == NodeStatus::Idle) {
            throw util::LogicError(
                    "StatefulActionNode::onRunning() must not return IDLE"
            );
        }
        return newNodeStatus;
    }
    return preNodeStatus;
}

void StatefulActionNode::Halt() {
    m_haltRequested.store(true);
    if(GetNodeStatus() == NodeStatus::Running) {
        OnHalted();
    }
    ResetNodeStatus();// might be redundant
}

NodeStatus behaviortree::ThreadedAction::ExecuteTick() {
    using LockType = std::unique_lock<std::mutex>;
    //send signal to other thread.
    // The other thread is in charge for changing the status
    if(GetNodeStatus() == NodeStatus::Idle) {
        SetNodeStatus(NodeStatus::Running);
        m_haltRequested = false;
        m_threadHandle = std::async(std::launch::async, [this]() {
            try {
                auto nodeStatus = Tick();
                if(!IsHaltRequested()) {
                    SetNodeStatus(nodeStatus);
                }
            } catch(std::exception &) {
                std::cerr << "\nUncaught exception from tick(): ["
                          << GetRegistrAtionName() << "/" << GetNodeName()
                          << "]\n"
                          << std::endl;
                // Set the exception pointer and the status atomically.
                LockType lock(m_mutex);
                m_exptr = std::current_exception();
                SetNodeStatus(behaviortree::NodeStatus::Idle);
            }
            EmitWakeUpSignal();
        });
    }

    LockType lock(m_mutex);
    if(m_exptr) {
        // The official interface of std::exception_ptr does not define any move
        // semantics. Thus, we copy and reset exptr_ manually.
        const auto exptrCopy = m_exptr;
        m_exptr = nullptr;
        std::rethrow_exception(exptrCopy);
    }
    return GetNodeStatus();
}

void ThreadedAction::Halt() {
    m_haltRequested.store(true);

    if(m_threadHandle.valid()) {
        m_threadHandle.wait();
    }
    m_threadHandle = {};
    ResetNodeStatus();// might be redundant
}
}// namespace behaviortree

// module behaviortree.action_node;
