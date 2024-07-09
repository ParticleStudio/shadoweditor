#include "behaviortree/tree_node.h"

#include <array>
#include <cstring>

namespace behaviortree {
struct TreeNode::PImpl {
    PImpl(std::string name, NodeConfig config)
        : name(std::move(name)), config(std::move(config)) {}

    const std::string name;

    NodeStatus nodeStatus{NodeStatus::IDLE};

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

    std::array<ScriptFunction, size_t(PreCond::COUNT)> preParsedArr;
    std::array<ScriptFunction, size_t(PostCond::COUNT)> postParsedArr;
};

TreeNode::TreeNode(std::string name, NodeConfig config)
    : m_P(new PImpl(std::move(name), std::move(config))) {}

TreeNode::TreeNode(TreeNode&& refOther) noexcept {
    this->m_P = std::move(refOther.m_P);
}

TreeNode& TreeNode::operator=(TreeNode&& refOther) noexcept {
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
        std::scoped_lock lk(m_P->callbackInjectionMutex);
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
        bool substituted = false;
        if(preTick && !IsStatusCompleted(m_P->nodeStatus)) {
            auto overrideNodeStatus = preTick(*this);
            if(IsStatusCompleted(overrideNodeStatus)) {
                // don't execute the actual tick()
                substituted = true;
                newNodeStatus = overrideNodeStatus;
            }
        }

        // Call the ACTUAL tick
        if(!substituted) {
            using namespace std::chrono;
            auto t1 = steady_clock::now();
            newNodeStatus = Tick();
            auto t2 = steady_clock::now();
            if(monitorTick) {
                monitorTick(*this, newNodeStatus, duration_cast<microseconds>(t2 - t1));
            }
        }
    }

    // injected post callback
    if(IsStatusCompleted(newNodeStatus)) {
        CheckPostConditions(newNodeStatus);
    }

    if(postTick) {
        auto overrideStatus = postTick(*this, newNodeStatus);
        if(IsStatusCompleted(overrideStatus)) {
            newNodeStatus = overrideStatus;
        }
    }

    // preserve the IDLE state if skipped, but communicate SKIPPED to parent
    if(newNodeStatus != NodeStatus::SKIPPED) {
        SetNodeStatus(newNodeStatus);
    }
    return newNodeStatus;
}

void TreeNode::HaltNode() {
    Halt();

    const auto& refParseExecutor = m_P->postParsedArr[size_t(PostCond::ON_HALTED)];
    if(refParseExecutor) {
        Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};
        refParseExecutor(env);
    }
}

void TreeNode::SetNodeStatus(NodeStatus newNodeStatus) {
    if(newNodeStatus == NodeStatus::IDLE) {
        throw RuntimeError("Node [", GetNodeName(),
                           "]: you are not allowed to set manually the status to IDLE. "
                           "If you know what you are doing (?) use resetStatus() instead.");
    }

    NodeStatus preNodeStatus;
    {
        std::unique_lock<std::mutex> uniqueLock(m_P->stateMutex);
        preNodeStatus = m_P->nodeStatus;
        m_P->nodeStatus = newNodeStatus;
    }
    if(preNodeStatus != newNodeStatus) {
        m_P->stateConditionVariable.notify_all();
        m_P->stateChangeSignal.notify(std::chrono::high_resolution_clock::now(), *this,
                                      preNodeStatus, newNodeStatus);
    }
}

TreeNode::PreScripts& TreeNode::PreConditionsScripts() {
    return m_P->preParsedArr;
}

TreeNode::PostScripts& TreeNode::PostConditionsScripts() {
    return m_P->postParsedArr;
}

Expected<NodeStatus> TreeNode::CheckPreConditions() {
    Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};

    // check the pre-conditions
    for(size_t index = 0; index < size_t(PreCond::COUNT); index++) {
        const auto& refParseExecutor = m_P->preParsedArr[index];
        if(!refParseExecutor) {
            continue;
        }

        const PreCond preId = PreCond(index);

        // Some preconditions are applied only when the node state is IDLE or SKIPPED
        if(m_P->nodeStatus == NodeStatus::IDLE || m_P->nodeStatus == NodeStatus::SKIPPED) {
            // what to do if the condition is true
            if(refParseExecutor(env).Cast<bool>()) {
                if(preId == PreCond::FAILURE_IF) {
                    return NodeStatus::FAILURE;
                } else if(preId == PreCond::SUCCESS_IF) {
                    return NodeStatus::SUCCESS;
                } else if(preId == PreCond::SKIP_IF) {
                    return NodeStatus::SKIPPED;
                }
            } else if(preId == PreCond::WHILE_TRUE) {// if the conditions is false
                return NodeStatus::SKIPPED;
            }
        } else if(m_P->nodeStatus == NodeStatus::RUNNING && preId == PreCond::WHILE_TRUE) {
            // what to do if the condition is false
            if(!refParseExecutor(env).Cast<bool>()) {
                HaltNode();
                return NodeStatus::SKIPPED;
            }
        }
    }
    return nonstd::make_unexpected("");// no precondition
}

void TreeNode::CheckPostConditions(NodeStatus nodeStatus) {
    auto ExecuteScript = [this](const PostCond& refCond) {
        const auto& refParseExecutor = m_P->postParsedArr[size_t(refCond)];
        if(refParseExecutor) {
            Ast::Environment env = {GetConfig().ptrBlackboard, GetConfig().ptrEnums};
            refParseExecutor(env);
        }
    };

    if(nodeStatus == NodeStatus::SUCCESS) {
        ExecuteScript(PostCond::ON_SUCCESS);
    } else if(nodeStatus == NodeStatus::FAILURE) {
        ExecuteScript(PostCond::ON_FAILURE);
    }
    ExecuteScript(PostCond::ALWAYS);
}

void TreeNode::ResetNodeStatus() {
    NodeStatus preNodeStatus;
    {
        std::unique_lock<std::mutex> lock(m_P->stateMutex);
        preNodeStatus = m_P->nodeStatus;
        m_P->nodeStatus = NodeStatus::IDLE;
    }

    if(preNodeStatus != NodeStatus::IDLE) {
        m_P->stateConditionVariable.notify_all();
        m_P->stateChangeSignal.notify(std::chrono::high_resolution_clock::now(), *this,
                                      preNodeStatus, NodeStatus::IDLE);
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

const std::string& TreeNode::GetNodeName() const {
    return m_P->name;
}

bool TreeNode::IsHalted() const {
    return m_P->nodeStatus == NodeStatus::IDLE;
}

TreeNode::StatusChangeSubscriber
TreeNode::SubscribeToStatusChange(TreeNode::StatusChangeCallback callback) {
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

const std::string& TreeNode::GetFullPath() const {
    return m_P->config.path;
}

const std::string& TreeNode::GetRegistrAtionName() const {
    return m_P->registrationId;
}

const NodeConfig& TreeNode::GetConfig() const {
    return m_P->config;
}

NodeConfig& TreeNode::GetConfig() {
    return m_P->config;
}

StringView TreeNode::GetRawPortValue(const std::string& refKey) const {
    auto ptrRemapIt = m_P->config.inputPortsMap.find(refKey);
    if(ptrRemapIt == m_P->config.inputPortsMap.end()) {
        ptrRemapIt = m_P->config.outputPortsMap.find(refKey);
        if(ptrRemapIt == m_P->config.outputPortsMap.end()) {
            throw std::logic_error(StrCat("[", refKey, "] not found"));
        }
    }
    return ptrRemapIt->second;
}

bool TreeNode::IsBlackboardPointer(StringView str, StringView* ptrStrippedPointer) {
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
        *ptrStrippedPointer = StringView(&str[frontIndex + 1], size - 2);
    }
    return valid;
}

StringView TreeNode::StripBlackboardPointer(StringView str) {
    StringView out;
    if(IsBlackboardPointer(str, &out)) {
        return out;
    }
    return {};
}

Expected<StringView> TreeNode::GetRemappedKey(StringView portName,
                                              StringView remappedPort) {
    if(remappedPort == "{=}" || remappedPort == "=") {
        return {portName};
    }
    StringView stripped;
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

void TreeNode::SetRegistrationId(StringView registrationId) {
    m_P->registrationId.assign(registrationId.data(), registrationId.size());
}

void TreeNode::SetWakeUpInstance(std::shared_ptr<WakeUpSignal> instance) {
    m_P->ptrWakeUp = instance;
}

void TreeNode::ModifyPortsRemapping(const PortsRemapping& refNewRemapping) {
    for(const auto& refNewIt: refNewRemapping) {
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
std::string ToStr<PreCond>(const PreCond& refPre) {
    switch(refPre) {
        case PreCond::SUCCESS_IF:
            return "_successIf";
        case PreCond::FAILURE_IF:
            return "_failureIf";
        case PreCond::SKIP_IF:
            return "_skipIf";
        case PreCond::WHILE_TRUE:
            return "_while";
        default:
            return "Undefined";
    }
}

template<>
std::string ToStr<PostCond>(const PostCond& refPre) {
    switch(refPre) {
        case PostCond::ON_SUCCESS:
            return "_onSuccess";
        case PostCond::ON_FAILURE:
            return "_onFailure";
        case PostCond::ALWAYS:
            return "_post";
        case PostCond::ON_HALTED:
            return "_onHalted";
        default:
            return "Undefined";
    }
}

AnyPtrLocked behaviortree::TreeNode::GetLockedPortContent(const std::string& refKey) {
    if(auto remappedKey = GetRemappedKey(refKey, GetRawPortValue(refKey))) {
        return m_P->config.ptrBlackboard->GetAnyLocked(std::string(*remappedKey));
    }
    return {};
}

}// namespace behaviortree
