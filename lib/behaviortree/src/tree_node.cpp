#include "behaviortree/tree_node.h"

#include <array>
#include <chrono>
#include <cstring>

namespace behaviortree {
struct TreeNode::PImpl {
    PImpl(std::string name, NodeConfig config): name(std::move(name)), config(std::move(config)) {}

    const std::string name;

    NodeStatus nodeStatus{NodeStatus::Idle};

    std::condition_variable stateConditionVariable;

    mutable std::mutex stateMutex;

    StatusChangeSignal stateChangeSignal;

    NodeConfig config;

    std::string registrationId;

    PreTickCallback preTickCallback;
    PostTickCallback postTickCallback;
    TickMonitorCallback tickMonitorCallback;

    std::mutex callbackInjectionMutex;

    std::shared_ptr<WakeUpSignal> pWakeUp;

    std::array<ScriptFunction, size_t(PreCond::Count)> preParsedArr;
    std::array<ScriptFunction, size_t(PostCond::Count)> postParsedArr;
};

TreeNode::TreeNode(std::string name, NodeConfig config): m_pPImpl(new PImpl(std::move(name), std::move(config))) {}

TreeNode::TreeNode(TreeNode &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
}

TreeNode &TreeNode::operator=(TreeNode &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
    return *this;
}

TreeNode::~TreeNode() {}

NodeStatus TreeNode::ExecuteTick() {
    auto newNodeStatus = m_pPImpl->nodeStatus;
    PreTickCallback preTick;
    PostTickCallback postTick;
    TickMonitorCallback monitorTick;

    {
        std::scoped_lock lock(m_pPImpl->callbackInjectionMutex);
        preTick = m_pPImpl->preTickCallback;
        postTick = m_pPImpl->postTickCallback;
        monitorTick = m_pPImpl->tickMonitorCallback;
    }

    // a pre-condition may return the new status.
    // In this case it override the actual tick()
    if(auto preCond = CheckPreConditions()) {
        newNodeStatus = preCond.value();
    } else {
        // injected pre-callback
        bool subStituted = false;
        if(preTick && !IsNodeStatusCompleted(m_pPImpl->nodeStatus)) {
            auto overrideNodeStatus = preTick(*this);
            if(IsNodeStatusCompleted(overrideNodeStatus)) {
                // don't execute the actual tick()
                subStituted = true;
                newNodeStatus = overrideNodeStatus;
            }
        }

        // Call the ACTUAL tick
        if(!subStituted) {
            auto beginTime = std::chrono::steady_clock::now();
            std::shared_ptr<void> executeLater(nullptr, [&](...) {
                if(monitorTick) {
                    auto endTime = std::chrono::steady_clock::now();
                    monitorTick(*this, newNodeStatus, duration_cast<std::chrono::microseconds>(endTime - beginTime));
                }
            });

            newNodeStatus = Tick();
        }
    }

    // injected post callback
    if(IsNodeStatusCompleted(newNodeStatus)) {
        CheckPostConditions(newNodeStatus);
    }

    if(postTick) {
        auto overrideNodeStatus = postTick(*this, newNodeStatus);
        if(IsNodeStatusCompleted(overrideNodeStatus)) {
            newNodeStatus = overrideNodeStatus;
        }
    }

    // preserve the IDLE state if skipped, but communicate SKIPPED to parent
    if(newNodeStatus != NodeStatus::Skipped) {
        SetNodeStatus(newNodeStatus);
    }
    return newNodeStatus;
}

void TreeNode::HaltNode() {
    Halt();

    const auto &rParseExecutor = m_pPImpl->postParsedArr[size_t(PostCond::OnHalted)];
    if(rParseExecutor) {
        Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
        rParseExecutor(env);
    }
}

void TreeNode::SetNodeStatus(NodeStatus newNodeStatus) {
    if(newNodeStatus == NodeStatus::Idle) {
        throw util::RuntimeError("Node [", GetNodeName(), "]: you are not allowed to set manually the status to IDLE. If you know what you are doing (?) use resetStatus() instead.");
    }

    NodeStatus preNodeStatus;
    {
        std::unique_lock<std::mutex> uniqueLock(m_pPImpl->stateMutex);
        preNodeStatus = m_pPImpl->nodeStatus;
        m_pPImpl->nodeStatus = newNodeStatus;
    }
    if(preNodeStatus != newNodeStatus) {
        m_pPImpl->stateConditionVariable.notify_all();
        m_pPImpl->stateChangeSignal.notify(std::chrono::high_resolution_clock::now(), *this, preNodeStatus, newNodeStatus);
    }
}

TreeNode::PreScripts &TreeNode::PreConditionsScripts() {
    return m_pPImpl->preParsedArr;
}

TreeNode::PostScripts &TreeNode::PostConditionsScripts() {
    return m_pPImpl->postParsedArr;
}

Expected<NodeStatus> TreeNode::CheckPreConditions() {
    Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};

    // check the pre-conditions
    for(size_t index = 0; index < size_t(PreCond::Count); index++) {
        const auto &rParseExecutor = m_pPImpl->preParsedArr[index];
        if(!rParseExecutor) {
            continue;
        }

        const PreCond preCond = PreCond(index);

        // Some preconditions are applied only when the node state is IDLE or SKIPPED
        if(m_pPImpl->nodeStatus == NodeStatus::Idle || m_pPImpl->nodeStatus == NodeStatus::Skipped) {
            // what to do if the condition is true
            if(rParseExecutor(env).Cast<bool>()) {
                switch(preCond) {
                    case PreCond::FailureIf: {
                        return NodeStatus::Failure;
                    } break;
                    case PreCond::SuccessIf: {
                        return NodeStatus::Success;
                    } break;
                    case PreCond::SkipIf: {
                        return NodeStatus::Skipped;
                    } break;
                    default: {
                    } break;
                }
            } else if(preCond == PreCond::WhileTrue) {// if the conditions is false
                return NodeStatus::Skipped;
            }
        } else if(m_pPImpl->nodeStatus == NodeStatus::Running && preCond == PreCond::WhileTrue) {
            // what to do if the condition is false
            if(!rParseExecutor(env).Cast<bool>()) {
                HaltNode();
                return NodeStatus::Skipped;
            }
        }
    }
    return nonstd::make_unexpected("");// no precondition
}

void TreeNode::CheckPostConditions(NodeStatus nodeStatus) {
    auto executeScript = [this](const PostCond &rPostCond) {
        const auto &rParseExecutor = m_pPImpl->postParsedArr[size_t(rPostCond)];
        if(rParseExecutor) {
            Ast::Environment env = {GetConfig().pBlackboard, GetConfig().pEnums};
            rParseExecutor(env);
        }
    };

    if(nodeStatus == NodeStatus::Success) {
        executeScript(PostCond::OnSuccess);
    } else if(nodeStatus == NodeStatus::Failure) {
        executeScript(PostCond::OnFailure);
    }
    executeScript(PostCond::Always);
}

void TreeNode::ResetNodeStatus() {
    NodeStatus preNodeStatus;
    {
        std::unique_lock<std::mutex> lock(m_pPImpl->stateMutex);
        preNodeStatus = m_pPImpl->nodeStatus;
        m_pPImpl->nodeStatus = NodeStatus::Idle;
    }

    if(preNodeStatus != NodeStatus::Idle) {
        m_pPImpl->stateConditionVariable.notify_all();
        m_pPImpl->stateChangeSignal.notify(std::chrono::high_resolution_clock::now(), *this, preNodeStatus, NodeStatus::Idle);
    }
}

NodeStatus TreeNode::GetNodeStatus() const {
    std::lock_guard<std::mutex> lock(m_pPImpl->stateMutex);
    return m_pPImpl->nodeStatus;
}

NodeStatus TreeNode::WaitValidStatus() {
    std::unique_lock<std::mutex> lock(m_pPImpl->stateMutex);

    while(IsHalted()) {
        m_pPImpl->stateConditionVariable.wait(lock);
    }
    return m_pPImpl->nodeStatus;
}

const std::string &TreeNode::GetNodeName() const {
    return m_pPImpl->name;
}

bool TreeNode::IsHalted() const {
    return m_pPImpl->nodeStatus == NodeStatus::Idle;
}

TreeNode::StatusChangeSubscriber TreeNode::SubscribeToStatusChange(TreeNode::StatusChangeCallback callback) {
    return m_pPImpl->stateChangeSignal.Subscribe(std::move(callback));
}

void TreeNode::SetPreTickFunction(PreTickCallback callback) {
    std::unique_lock lock(m_pPImpl->callbackInjectionMutex);
    m_pPImpl->preTickCallback = callback;
}

void TreeNode::SetPostTickFunction(PostTickCallback callback) {
    std::unique_lock lock(m_pPImpl->callbackInjectionMutex);
    m_pPImpl->postTickCallback = callback;
}

void TreeNode::SetTickMonitorCallback(TickMonitorCallback callback) {
    std::unique_lock lock(m_pPImpl->callbackInjectionMutex);
    m_pPImpl->tickMonitorCallback = callback;
}

uint16_t TreeNode::GetUid() const {
    return m_pPImpl->config.uid;
}

const std::string &TreeNode::GetFullPath() const {
    return m_pPImpl->config.path;
}

const std::string &TreeNode::GetRegistrAtionName() const {
    return m_pPImpl->registrationId;
}

const NodeConfig &TreeNode::GetConfig() const {
    return m_pPImpl->config;
}

NodeConfig &TreeNode::GetConfig() {
    return m_pPImpl->config;
}

std::string_view TreeNode::GetRawPortValue(const std::string &rKey) const {
    auto remapIter = m_pPImpl->config.inputPortMap.find(rKey);
    if(remapIter == m_pPImpl->config.inputPortMap.end()) {
        remapIter = m_pPImpl->config.outputPortMap.find(rKey);
        if(remapIter == m_pPImpl->config.outputPortMap.end()) {
            throw std::logic_error(util::StrCat("[", rKey, "] not found"));
        }
    }
    return remapIter->second;
}

bool TreeNode::IsBlackboardPointer(std::string_view str, std::string_view *pStrippedPointer) {
    if(str.size() < 3) {
        return false;
    }
    // strip leading and following spaces
    size_t frontIndex = 0;
    size_t lastIndex = str.size() - 1;
    while(str[frontIndex] == ' ' && frontIndex <= lastIndex) {
        frontIndex++;
    }
    while(str[lastIndex] == ' ' && frontIndex <= lastIndex) {
        lastIndex--;
    }
    const auto size = (lastIndex - frontIndex) + 1;
    auto valid = size >= 3 && str[frontIndex] == '{' && str[lastIndex] == '}';
    if(valid && pStrippedPointer) {
        *pStrippedPointer = std::string_view(&str[frontIndex + 1], size - 2);
    }
    return valid;
}

std::string_view TreeNode::StripBlackboardPointer(std::string_view str) {
    std::string_view out;
    if(IsBlackboardPointer(str, &out)) {
        return out;
    }
    return {};
}

Expected<std::string_view> TreeNode::GetRemappedKey(std::string_view portName, std::string_view remappedPort) {
    if(remappedPort == "{=}" || remappedPort == "=") {
        return {portName};
    }
    std::string_view stripped;
    if(IsBlackboardPointer(remappedPort, &stripped)) {
        return {stripped};
    }
    return nonstd::make_unexpected("Not a blackboard pointer");
}

void TreeNode::EmitWakeUpSignal() {
    if(m_pPImpl->pWakeUp) {
        m_pPImpl->pWakeUp->EmitSignal();
    }
}

bool TreeNode::RequiresWakeUp() const {
    return bool(m_pPImpl->pWakeUp);
}

void TreeNode::SetRegistrationId(std::string_view registrationId) {
    m_pPImpl->registrationId.assign(registrationId.data(), registrationId.size());
}

void TreeNode::SetWakeUpInstance(std::shared_ptr<WakeUpSignal> pInstance) {
    m_pPImpl->pWakeUp = pInstance;
}

void TreeNode::ModifyPortsRemapping(const PortsRemapping &rNewRemapping) {
    for(const auto &newIter: rNewRemapping) {
        auto iter = m_pPImpl->config.inputPortMap.find(newIter.first);
        if(iter != m_pPImpl->config.inputPortMap.end()) {
            iter->second = newIter.second;
        }
        iter = m_pPImpl->config.outputPortMap.find(newIter.first);
        if(iter != m_pPImpl->config.outputPortMap.end()) {
            iter->second = newIter.second;
        }
    }
}

template<>
std::string ToStr<PreCond>(const PreCond &rPreCond) {
    switch(rPreCond) {
        case PreCond::SuccessIf: {
            return "_successIf";
        } break;
        case PreCond::FailureIf: {
            return "_failureIf";
        } break;
        case PreCond::SkipIf: {
            return "_skipIf";
        } break;
        case PreCond::WhileTrue: {
            return "_while";
        } break;
        default: {
            return "Undefined";
        } break;
    }
}

template<>
std::string ToStr<PostCond>(const PostCond &rPreCond) {
    switch(rPreCond) {
        case PostCond::OnSuccess: {
            return "_onSuccess";
        } break;
        case PostCond::OnFailure: {
            return "_onFailure";
        } break;
        case PostCond::Always: {
            return "_post";
        } break;
        case PostCond::OnHalted: {
            return "_onHalted";
        } break;
        default: {
            return "Undefined";
        } break;
    }
}

AnyPtrLocked behaviortree::TreeNode::GetLockedPortContent(const std::string &rKey) {
    if(auto remappedKey = GetRemappedKey(rKey, GetRawPortValue(rKey))) {
        return m_pPImpl->config.pBlackboard->GetAnyLocked(std::string(*remappedKey));
    }
    return {};
}

}// namespace behaviortree
