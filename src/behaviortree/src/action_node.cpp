#define MINICORO_IMPL
#include "behaviortree/action_node.h"

#include "minicoro/minicoro.h"

using namespace behaviortree;

ActionNodeBase::ActionNodeBase(const std::string& refName, const NodeConfig& refConfig)
    : LeafNode::LeafNode(refName, refConfig) {}

//-------------------------------------------------------

SimpleActionNode::SimpleActionNode(const std::string& refName,
                                   SimpleActionNode::TickFunctor tickFunctor,
                                   const NodeConfig& refConfig)
    : SyncActionNode(refName, refConfig), m_TickFunctor(std::move(tickFunctor)) {}

NodeStatus SimpleActionNode::Tick() {
    NodeStatus prevStatus = NodeStatus();

    if(prevStatus == NodeStatus::IDLE) {
        SetStatus(NodeStatus::RUNNING);
        prevStatus = NodeStatus::RUNNING;
    }

    NodeStatus status = m_TickFunctor(*this);
    if(status != prevStatus) {
        SetStatus(status);
    }
    return status;
}

//-------------------------------------------------------

SyncActionNode::SyncActionNode(const std::string& name, const NodeConfig& config)
    : ActionNodeBase(name, config) {}

NodeStatus SyncActionNode::executeTick() {
    auto stat = ActionNodeBase::executeTick();
    if(stat == NodeStatus::RUNNING) {
        throw LogicError("SyncActionNode MUST never return RUNNING");
    }
    return stat;
}

//-------------------------------------

struct CoroActionNode::Pimpl {
    mco_coro* coro = nullptr;
    mco_desc desc;
};

void CoroEntry(mco_coro* co) {
    static_cast<CoroActionNode*>(co->user_data)->tickImpl();
}

CoroActionNode::CoroActionNode(const std::string& name, const NodeConfig& config)
    : ActionNodeBase(name, config), _p(new Pimpl) {}

CoroActionNode::~CoroActionNode() {
    destroyCoroutine();
}

void CoroActionNode::setStatusRunningAndYield() {
    setStatus(NodeStatus::RUNNING);
    mco_yield(_p->coro);
}

NodeStatus CoroActionNode::executeTick() {
    // create a new coroutine, if necessary
    if(_p->coro == nullptr) {
        // First initialize a `desc` object through `mco_desc_init`.
        _p->desc = mco_desc_init(CoroEntry, 0);
        _p->desc.user_data = this;

        mco_result res = mco_create(&_p->coro, &_p->desc);
        if(res != MCO_SUCCESS) {
            throw RuntimeError("Can't create coroutine");
        }
    }

    //------------------------
    // execute the coroutine
    mco_resume(_p->coro);
    //------------------------

    // check if the coroutine finished. In this case, destroy it
    if(mco_status(_p->coro) == MCO_DEAD) {
        destroyCoroutine();
    }

    return status();
}

void CoroActionNode::tickImpl() {
    setStatus(TreeNode::executeTick());
}

void CoroActionNode::halt() {
    destroyCoroutine();
    resetStatus();// might be redundant
}

void CoroActionNode::destroyCoroutine() {
    if(_p->coro) {
        mco_result res = mco_destroy(_p->coro);
        if(res != MCO_SUCCESS) {
            throw RuntimeError("Can't destroy coroutine");
        }
        _p->coro = nullptr;
    }
}

bool StatefulActionNode::isHaltRequested() const {
    return halt_requested_.load();
}

NodeStatus StatefulActionNode::tick() {
    const NodeStatus prev_status = status();

    if(prev_status == NodeStatus::IDLE) {
        NodeStatus new_status = onStart();
        if(new_status == NodeStatus::IDLE) {
            throw LogicError("StatefulActionNode::onStart() must not return IDLE");
        }
        return new_status;
    }
    //------------------------------------------
    if(prev_status == NodeStatus::RUNNING) {
        NodeStatus new_status = onRunning();
        if(new_status == NodeStatus::IDLE) {
            throw LogicError("StatefulActionNode::onRunning() must not return IDLE");
        }
        return new_status;
    }
    return prev_status;
}

void StatefulActionNode::halt() {
    halt_requested_.store(true);
    if(status() == NodeStatus::RUNNING) {
        onHalted();
    }
    resetStatus();// might be redundant
}

NodeStatus BT::ThreadedAction::executeTick() {
    using lock_type = std::unique_lock<std::mutex>;
    //send signal to other thread.
    // The other thread is in charge for changing the status
    if(status() == NodeStatus::IDLE) {
        setStatus(NodeStatus::RUNNING);
        halt_requested_ = false;
        thread_handle_ = std::async(std::launch::async, [this]() {
            try {
                auto status = tick();
                if(!isHaltRequested()) {
                    setStatus(status);
                }
            } catch(std::exception&) {
                std::cerr << "\nUncaught exception from tick(): [" << registrationName() << "/"
                          << name() << "]\n"
                          << std::endl;
                // Set the exception pointer and the status atomically.
                lock_type l(mutex_);
                exptr_ = std::current_exception();
                setStatus(BT::NodeStatus::IDLE);
            }
            emitWakeUpSignal();
        });
    }

    lock_type l(mutex_);
    if(exptr_) {
        // The official interface of std::exception_ptr does not define any move
        // semantics. Thus, we copy and reset exptr_ manually.
        const auto exptr_copy = exptr_;
        exptr_ = nullptr;
        std::rethrow_exception(exptr_copy);
    }
    return status();
}

void ThreadedAction::halt() {
    halt_requested_.store(true);

    if(thread_handle_.valid()) {
        thread_handle_.wait();
    }
    thread_handle_ = {};
    resetStatus();// might be redundant
}
