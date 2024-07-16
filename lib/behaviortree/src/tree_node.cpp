#include "behaviortree/tree_node.h"

#include <array>
#include <cstring>

namespace behaviortree {
struct TreeNode::PImpl {
    PImpl(std::string name, NodeConfig config): name(std::move(name)),
                                                config(std::move(config)) {}

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

    std::shared_ptr<WakeUpSignal> ptrWakeUp;

    std::array<ScriptFunction, size_t(PreCond::Count)> preParsedArr;
    std::array<ScriptFunction, size_t(PostCond::Count)> postParsedArr;
};

TreeNode::TreeNode(std::string name, NodeConfig config): m_P(new PImpl(std::move(name), std::move(config))) {}

TreeNode::TreeNode(TreeNode &&refOther) noexcept {
    this->m_P = std::move(refOther.m_P);
}

TreeNode &TreeNode::operator=(TreeNode &&refOther) noexcept {
    this->m_P = std::move(refOther.m_P);
    return *this;
}

TreeNode::~TreeNode() {}

NodeStatus TreeNode::ExecuteTick() {
    auto newNodeStatus = m_P->nodeStatus;
    PreTickCallback preTick;
    PostTickCallback postTick;
    TickMonitorCallback monitorTick;
    {
        std::scoped_lock lock(m_P->callbackInjectionMutex);
        preTick = m_P->preTickCallback;
        postTick = m_P->postTickCallback;
        monitorTick = m_P->tickMonitorCallback;
    }

    // a pre-condition may return the new status.
    // In this case it override the actual tick()
    if(auto preCond = CheckPreConditions()) {
        newNodeStatus = preCond.value();
    } else {
        // injected pre-callback
        bool subStituted = false;
        if(preTick && !IsNodeStatusCompleted(m_P->nodeStatus)) {
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
            newNodeStatus = Tick();
            auto endTime = std::chrono::steady_clock::now();
            if(monitorTick) {
                monitorTick(*this, newNodeStatus, duration_cast<std::chrono::microseconds>(endTime - beginTime));
            }
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

    const auto &refParseExecutor = m_P->postParsedArr[size_t(PostCond::OnHalted)];
    if(refParseExecutor) {
        Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};
        refParseExecutor(env);
    }
}

void TreeNode::SetNodeStatus(NodeStatus newNodeStatus) {
    if(newNodeStatus == NodeStatus::Idle) {
        throw RuntimeError(
                "Node [", GetNodeName(),
                "]: you are not allowed to set manually the status to IDLE. "
                "If you know what you are doing (?) use resetStatus() instead."
        );
    }

    NodeStatus preNodeStatus;
    {
        std::unique_lock<std::mutex> uniqueLock(m_P->stateMutex);
        preNodeStatus = m_P->nodeStatus;
        m_P->nodeStatus = newNodeStatus;
    }
    if(preNodeStatus != newNodeStatus) {
        m_P->stateConditionVariable.notify_all();
        m_P->stateChangeSignal.notify(std::chrono::high_resolution_clock::now(), *this, preNodeStatus, newNodeStatus);
    }
}

TreeNode::PreScripts &TreeNode::PreConditionsScripts() {
    return m_P->preParsedArr;
}

TreeNode::PostScripts &TreeNode::PostConditionsScripts() {
    return m_P->postParsedArr;
}

Expected<NodeStatus> TreeNode::CheckPreConditions() {
    Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};

    // check the pre-conditions
    for(size_t index = 0; index < size_t(PreCond::Count); index++) {
        const auto &refParseExecutor = m_P->preParsedArr[index];
        if(!refParseExecutor) {
            continue;
        }

        const PreCond preCond = PreCond(index);

        // Some preconditions are applied only when the node state is IDLE or SKIPPED
        if(m_P->nodeStatus == NodeStatus::Idle ||
           m_P->nodeStatus == NodeStatus::Skipped) {
            // what to do if the condition is true
            if(refParseExecutor(env).Cast<bool>()) {
                switch(preCond) {
                    case PreCond::FailureIf: {
                        return NodeStatus::Failure;
                    }
                    case PreCond::SuccessIf: {
                        return NodeStatus::Success;
                    }
                    case PreCond::SkipIf: {
                        return NodeStatus::Skipped;
                    }
                    default: {
                    } break;
                }
            } else if(preCond == PreCond::WhileTrue) {// if the conditions is false
                return NodeStatus::Skipped;
            }
        } else if(m_P->nodeStatus == NodeStatus::Running && preCond == PreCond::WhileTrue) {
            // what to do if the condition is false
            if(!refParseExecutor(env).Cast<bool>()) {
                HaltNode();
                return NodeStatus::Skipped;
            }
        }
    }
    return nonstd::make_unexpected("");// no precondition
}

void TreeNode::CheckPostConditions(NodeStatus nodeStatus) {
    auto executeScript = [this](const PostCond &refCond) {
        const auto &refParseExecutor = m_P->postParsedArr[size_t(refCond)];
        if(refParseExecutor) {
            Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};
            refParseExecutor(env);
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
        std::unique_lock<std::mutex> lock(m_P->stateMutex);
        preNodeStatus = m_P->nodeStatus;
        m_P->nodeStatus = NodeStatus::Idle;
    }

    if(preNodeStatus != NodeStatus::Idle) {
        m_P->stateConditionVariable.notify_all();
        m_P->stateChangeSignal.notify(
                std::chrono::high_resolution_clock::now(), *this, preNodeStatus,
                NodeStatus::Idle
        );
    }
}

NodeStatus TreeNode::GetNodeStatus() const {
    std::lock_guard<std::mutex> lock(m_P->stateMutex);
    return m_P->nodeStatus;
}

NodeStatus TreeNode::WaitValidStatus() {
    std::unique_lock<std::mutex> lock(m_P->stateMutex);

    while(IsHalted()) {
        m_P->stateConditionVariable.wait(lock);
    }
    return m_P->nodeStatus;
}

const std::string &TreeNode::GetNodeName() const {
    return m_P->name;
}

bool TreeNode::IsHalted() const {
    return m_P->nodeStatus == NodeStatus::Idle;
}

TreeNode::StatusChangeSubscriber TreeNode::SubscribeToStatusChange(
        TreeNode::StatusChangeCallback callback
) {
    return m_P->stateChangeSignal.Subscribe(std::move(callback));
}

void TreeNode::SetPreTickFunction(PreTickCallback callback) {
    std::unique_lock lk(m_P->callbackInjectionMutex);
    m_P->preTickCallback = callback;
}

void TreeNode::SetPostTickFunction(PostTickCallback callback) {
    std::unique_lock lk(m_P->callbackInjectionMutex);
    m_P->postTickCallback = callback;
}

void TreeNode::SetTickMonitorCallback(TickMonitorCallback callback) {
    std::unique_lock lk(m_P->callbackInjectionMutex);
    m_P->tickMonitorCallback = callback;
}

uint16_t TreeNode::GetUID() const {
    return m_P->config.uid;
}

const std::string &TreeNode::GetFullPath() const {
    return m_P->config.path;
}

const std::string &TreeNode::GetRegistrAtionName() const {
    return m_P->registrationId;
}

const NodeConfig &TreeNode::GetConfig() const {
    return m_P->config;
}

NodeConfig &TreeNode::GetConfig() {
    return m_P->config;
}

std::string_view TreeNode::GetRawPortValue(const std::string &refKey) const {
    auto ptrRemapIt = m_P->config.inputPortsMap.find(refKey);
    if(ptrRemapIt == m_P->config.inputPortsMap.end()) {
        ptrRemapIt = m_P->config.outputPortsMap.find(refKey);
        if(ptrRemapIt == m_P->config.outputPortsMap.end()) {
            throw std::logic_error(StrCat("[", refKey, "] not found"));
        }
    }
    return ptrRemapIt->second;
}

bool TreeNode::IsBlackboardPointer(std::string_view str, std::string_view *ptrStrippedPointer) {
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
    if(valid && ptrStrippedPointer) {
        *ptrStrippedPointer = std::string_view(&str[frontIndex + 1], size - 2);
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
    if(m_P->ptrWakeUp) {
        m_P->ptrWakeUp->EmitSignal();
    }
}

bool TreeNode::RequiresWakeUp() const {
    return bool(m_P->ptrWakeUp);
}

void TreeNode::SetRegistrationId(std::string_view registrationId) {
    m_P->registrationId.assign(registrationId.data(), registrationId.size());
}

void TreeNode::SetWakeUpInstance(std::shared_ptr<WakeUpSignal> instance) {
    m_P->ptrWakeUp = instance;
}

void TreeNode::ModifyPortsRemapping(const PortsRemapping &refNewRemapping) {
    for(const auto &refNewIt: refNewRemapping) {
        auto it = m_P->config.inputPortsMap.find(refNewIt.first);
        if(it != m_P->config.inputPortsMap.end()) {
            it->second = refNewIt.second;
        }
        it = m_P->config.outputPortsMap.find(refNewIt.first);
        if(it != m_P->config.outputPortsMap.end()) {
            it->second = refNewIt.second;
        }
    }
}

template<>
std::string ToStr<PreCond>(const PreCond &refPre) {
    switch(refPre) {
        case PreCond::SuccessIf:
            return "_successIf";
        case PreCond::FailureIf:
            return "_failureIf";
        case PreCond::SkipIf:
            return "_skipIf";
        case PreCond::WhileTrue:
            return "_while";
        default:
            return "Undefined";
    }
}

template<>
std::string ToStr<PostCond>(const PostCond &refPre) {
    switch(refPre) {
        case PostCond::OnSuccess:
            return "_onSuccess";
        case PostCond::OnFailure:
            return "_onFailure";
        case PostCond::Always:
            return "_post";
        case PostCond::OnHalted:
            return "_onHalted";
        default:
            return "Undefined";
    }
}

AnyPtrLocked behaviortree::TreeNode::GetLockedPortContent(const std::string &refKey) {
    if(auto remappedKey = GetRemappedKey(refKey, GetRawPortValue(refKey))) {
        return m_P->config.ptrBlackboard->GetAnyLocked(std::string(*remappedKey)
        );
    }
    return {};
}

}// namespace behaviortree
