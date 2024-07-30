#include "behaviortree/factory.h"

#include <filesystem>

#include "behaviortree/json_parsing.h"
#include "behaviortree/util/shared_library.h"
#include "behaviortree/util/wildcards.hpp"
#include "nlohmann/json.hpp"

namespace behaviortree {
bool WildcardMatch(std::string const &rStr, std::string_view filter) {
    return wildcards::match(rStr, filter);
}

struct BehaviorTreeFactory::PImpl {
    std::unordered_map<std::string, NodeBuilder> builderMap;
    std::unordered_map<std::string, TreeNodeManifest> manifestMap;
    std::set<std::string> builtinIdSet;
    std::unordered_map<std::string, Any> behaviortreeDefinitionsMap;
    std::shared_ptr<std::unordered_map<std::string, int>> pScriptingEnums;
    std::shared_ptr<behaviortree::Parser> pParser;
    std::unordered_map<std::string, SubstitutionRule> substitutionRulesMap;
};

BehaviorTreeFactory::BehaviorTreeFactory(): m_pPImpl(new PImpl) {
    m_pPImpl->pParser = std::make_shared<JsonParser>(*this);
    RegisterNodeType<FallbackNode>("Fallback");
    RegisterNodeType<FallbackNode>("AsyncFallback", true);
    RegisterNodeType<SequenceNode>("Sequence");
    RegisterNodeType<SequenceNode>("AsyncSequence", true);
    RegisterNodeType<SequenceWithMemory>("SequenceWithMemory");

    RegisterNodeType<ParallelNode>("Parallel");
    RegisterNodeType<ParallelAllNode>("ParallelAll");
    RegisterNodeType<ReactiveSequence>("ReactiveSequence");
    RegisterNodeType<ReactiveFallback>("ReactiveFallback");
    RegisterNodeType<IfThenElseNode>("IfThenElse");
    RegisterNodeType<WhileDoElseNode>("WhileDoElse");

    RegisterNodeType<InverterNode>("Inverter");

    RegisterNodeType<RetryNode>("RetryUntilSuccessful");
    RegisterNodeType<KeepRunningUntilFailureNode>("KeepRunningUntilFailure");
    RegisterNodeType<RepeatNode>("Repeat");
    RegisterNodeType<TimeoutNode>("Timeout");
    RegisterNodeType<DelayNode>("Delay");
    RegisterNodeType<RunOnceNode>("RunOnce");

    RegisterNodeType<ForceSuccessNode>("ForceSuccess");
    RegisterNodeType<ForceFailureNode>("ForceFailure");

    RegisterNodeType<AlwaysSuccessNode>("AlwaysSuccess");
    RegisterNodeType<AlwaysFailureNode>("AlwaysFailure");
    RegisterNodeType<ScriptNode>("Script");
    RegisterNodeType<ScriptCondition>("ScriptCondition");
    RegisterNodeType<SetBlackboardNode>("SetBlackboard");
    RegisterNodeType<SleepNode>("Sleep");
    RegisterNodeType<UnsetBlackboardNode>("UnsetBlackboard");

    RegisterNodeType<SubtreeNode>("Subtree");

    RegisterNodeType<PreconditionNode>("Precondition");

    RegisterNodeType<SwitchNode<2>>("Switch2");
    RegisterNodeType<SwitchNode<3>>("Switch3");
    RegisterNodeType<SwitchNode<4>>("Switch4");
    RegisterNodeType<SwitchNode<5>>("Switch5");
    RegisterNodeType<SwitchNode<6>>("Switch6");

    RegisterNodeType<LoopNode<int>>("LoopInt");
    RegisterNodeType<LoopNode<bool>>("LoopBool");
    RegisterNodeType<LoopNode<double>>("LoopDouble");
    RegisterNodeType<LoopNode<std::string>>("LoopString");

    RegisterNodeType<EntryUpdatedAction>("WasEntryUpdated");
    RegisterNodeType<EntryUpdatedDecorator>("SkipUnlessUpdated", NodeStatus::Skipped);
    RegisterNodeType<EntryUpdatedDecorator>("WaitValueUpdate", NodeStatus::Running);

    for(const auto &rIter: m_pPImpl->builderMap) {
        m_pPImpl->builtinIdSet.insert(rIter.first);
    }

    m_pPImpl->pScriptingEnums = std::make_shared<std::unordered_map<std::string, int>>();
}

BehaviorTreeFactory::BehaviorTreeFactory(BehaviorTreeFactory &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
}

BehaviorTreeFactory &BehaviorTreeFactory::operator=(BehaviorTreeFactory &&rOther) noexcept {
    this->m_pPImpl = std::move(rOther.m_pPImpl);
    return *this;
}

BehaviorTreeFactory::~BehaviorTreeFactory() = default;

bool BehaviorTreeFactory::UnregisterBuilder(const std::string &rId) {
    if(GetBuiltinNodes().count(rId)) {
        throw LogicError("You can not remove the builtin registration ID [", rId, "]");
    }
    auto iter = m_pPImpl->builderMap.find(rId);
    if(iter == m_pPImpl->builderMap.end()) {
        return false;
    }
    m_pPImpl->builderMap.erase(rId);
    m_pPImpl->manifestMap.erase(rId);
    return true;
}

void BehaviorTreeFactory::RegisterBuilder(const TreeNodeManifest &rManifest, const NodeBuilder &rBuilder) {
    auto iter = m_pPImpl->builderMap.find(rManifest.registrationId);
    if(iter != m_pPImpl->builderMap.end()) {
        throw BehaviorTreeException("Id [", rManifest.registrationId, "] already registered");
    }

    m_pPImpl->builderMap.insert({rManifest.registrationId, rBuilder});
    m_pPImpl->manifestMap.insert({rManifest.registrationId, rManifest});
}

void BehaviorTreeFactory::RegisterSimpleCondition(const std::string &rName, const SimpleConditionNode::TickFunctor &rTickFunctor, PortMap portMap) {
    NodeBuilder builder = [rName, rTickFunctor](const std::string &refName, const NodeConfig &rNodeConfig) {
        return std::make_unique<SimpleConditionNode>(refName, rTickFunctor, rNodeConfig);
    };

    TreeNodeManifest manifest = {NodeType::Condition, rName, std::move(portMap), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterSimpleAction(const std::string &rName, const SimpleActionNode::TickFunctor &rTickFunctor, PortMap portMap) {
    NodeBuilder builder = [rName, rTickFunctor](const std::string &refName, const NodeConfig &refConfig) {
        return std::make_unique<SimpleActionNode>(refName, rTickFunctor, refConfig);
    };

    TreeNodeManifest manifest = {NodeType::Action, rName, std::move(portMap), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterSimpleDecorator(const std::string &rName, const SimpleDecoratorNode::TickFunctor &rConfig, PortMap portMap) {
    NodeBuilder builder = [rConfig, rName](const std::string &refName, const NodeConfig &refConfig) {
        return std::make_unique<SimpleDecoratorNode>(refName, rConfig, refConfig);
    };

    TreeNodeManifest manifest = {NodeType::Decorator, rName, std::move(portMap), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterFromPlugin(const std::string &rFilePath) {
    behaviortree::SharedLibrary loader;
    loader.Load(rFilePath);
    typedef void (*Func)(BehaviorTreeFactory &);

    if(loader.HasSymbol(PLUGIN_SYMBOL)) {
        Func func = static_cast<Func>(loader.GetSymbol(PLUGIN_SYMBOL));
        func(*this);
    } else {
        std::cout << "ERROR loading library [" << rFilePath << "]: can't find symbol [" << PLUGIN_SYMBOL << "]" << std::endl;
    }
}

void BehaviorTreeFactory::RegisterBehaviorTreeFromFile(const std::filesystem::path &rFileName) {
    m_pPImpl->pParser->LoadFromFile(rFileName);
}

void BehaviorTreeFactory::RegisterBehaviorTreeFromText(const std::string &rJsonText) {
    m_pPImpl->pParser->LoadFromText(rJsonText);
}

std::vector<std::string> BehaviorTreeFactory::RegisteredBehaviorTrees() const {
    return m_pPImpl->pParser->RegisteredBehaviorTrees();
}

void BehaviorTreeFactory::ClearRegisteredBehaviorTrees() {
    m_pPImpl->pParser->ClearInternalState();
}

std::unique_ptr<TreeNode> BehaviorTreeFactory::InstantiateTreeNode(const std::string &rName, const std::string &rId, const NodeConfig &rConfig) const {
    auto idNotFound = [this, rId] {
        std::cerr << rId << " not included in this list:" << std::endl;
        for(const auto &rBuilderIter: m_pPImpl->builderMap) {
            std::cerr << rBuilderIter.first << std::endl;
        }
        throw RuntimeError("BehaviorTreeFactory: ID [", rId, "] not registered");
    };

    auto manifestIter = m_pPImpl->manifestMap.find(rId);
    if(manifestIter == m_pPImpl->manifestMap.end()) {
        idNotFound();
    }

    std::unique_ptr<TreeNode> node;

    bool substituted = false;
    for(const auto &[filter, rule]: m_pPImpl->substitutionRulesMap) {
        if(filter == rName || filter == rId || wildcards::match(rConfig.path, filter)) {
            // first case: the rule is simply a string with the name of the
            // node to create instead
            if(const auto pSbstitutedId = std::get_if<std::string>(&rule)) {
                auto builderIter = m_pPImpl->builderMap.find(*pSbstitutedId);
                if(builderIter != m_pPImpl->builderMap.end()) {
                    auto &rBuilder = builderIter->second;
                    node = rBuilder(rName, rConfig);
                } else {
                    throw RuntimeError("Substituted Node ID [", *pSbstitutedId, "] not found");
                }
                substituted = true;
                break;
            } else if(const auto pTestConfig = std::get_if<TestNodeConfig>(&rule)) {
                // second case, the varian is a TestNodeConfig
                auto pTestNode = new TestNode(rName, rConfig, *pTestConfig);
                node.reset(pTestNode);
                substituted = true;
                break;
            }
        }
    }

    // No substitution rule applied: default behavior
    if(!substituted) {
        auto it_builder = m_pPImpl->builderMap.find(rId);
        if(it_builder == m_pPImpl->builderMap.end()) {
            idNotFound();
        }
        auto &rBuilder = it_builder->second;
        node = rBuilder(rName, rConfig);
    }

    node->SetRegistrationId(rId);
    node->GetConfig().pEnums = m_pPImpl->pScriptingEnums;

    auto assignConditions = [](auto &rConditions, auto &rExecutors) {
        for(const auto &[rCondId, rScript]: rConditions) {
            if(auto executor = ParseScript(rScript)) {
                rExecutors[size_t(rCondId)] = executor.value();
            } else {
                throw LogicError("Error in the script \"", rScript, "\"\n", executor.error());
            }
        }
    };
    assignConditions(rConfig.preConditionMap, node->PreConditionsScripts());
    assignConditions(rConfig.postConditionMap, node->PostConditionsScripts());

    return node;
}

const std::unordered_map<std::string, NodeBuilder> &BehaviorTreeFactory::GetBuilder() const {
    return m_pPImpl->builderMap;
}

const std::unordered_map<std::string, TreeNodeManifest> &BehaviorTreeFactory::GetManifest() const {
    return m_pPImpl->manifestMap;
}

const std::set<std::string> &BehaviorTreeFactory::GetBuiltinNodes() const {
    return m_pPImpl->builtinIdSet;
}

Tree BehaviorTreeFactory::CreateTreeFromText(const std::string &rText, const Blackboard::Ptr &pBlackboard) {
    if(!m_pPImpl->pParser->RegisteredBehaviorTrees().empty()) {
        std::cout << "WARNING: You executed "
                     "BehaviorTreeFactory::CreateTreeFromText "
                     "after registerBehaviorTreeFrom[File/Text].\n"
                     "This is NOT, probably, what you want to do.\n"
                     "You should probably use BehaviorTreeFactory::CreateTree, "
                     "instead"
                  << std::endl;
    }
    JsonParser parser(*this);
    parser.LoadFromText(rText);
    auto tree = parser.InstantiateTree(pBlackboard);
    tree.m_manifestsMap = this->GetManifest();
    return tree;
}

Tree BehaviorTreeFactory::CreateTreeFromFile(const std::filesystem::path &rFilePath, const Blackboard::Ptr &pBlackboard) {
    if(!m_pPImpl->pParser->RegisteredBehaviorTrees().empty()) {
        std::cout << "WARNING: You executed "
                     "BehaviorTreeFactory::CreateTreeFromFile "
                     "after registerBehaviorTreeFrom[File/Text].\n"
                     "This is NOT, probably, what you want to do.\n"
                     "You should probably use BehaviorTreeFactory::CreateTree, "
                     "instead"
                  << std::endl;
    }

    JsonParser parser(*this);
    parser.LoadFromFile(rFilePath);
    auto tree = parser.InstantiateTree(pBlackboard);
    tree.m_manifestsMap = this->GetManifest();
    return tree;
}

Tree BehaviorTreeFactory::CreateTree(const std::string &rTreeName, Blackboard::Ptr pBlackboard) {
    auto tree = m_pPImpl->pParser->InstantiateTree(pBlackboard, rTreeName);
    tree.m_manifestsMap = this->GetManifest();
    return tree;
}

void BehaviorTreeFactory::AddMetadataToManifest(const std::string &rNodeId, const MetedataVec &rMetadata) {
    auto iter = m_pPImpl->manifestMap.find(rNodeId);
    if(iter == m_pPImpl->manifestMap.end()) {
        throw std::runtime_error("AddMetadataToManifest: wrong ID");
    }
    iter->second.metadataVec = rMetadata;
}

void BehaviorTreeFactory::RegisterScriptingEnum(std::string_view name, int value) {
    const auto str = std::string(name);
    auto iter = m_pPImpl->pScriptingEnums->find(str);
    if(iter == m_pPImpl->pScriptingEnums->end()) {
        m_pPImpl->pScriptingEnums->insert({str, value});
    } else {
        if(iter->second != value) {
            throw LogicError(
                    StrCat("Registering the enum [", name,
                           "] twice with different values, first ",
                           std::to_string(iter->second), " and later ",
                           std::to_string(value))
            );
        }
    }
}

void BehaviorTreeFactory::ClearSubstitutionRules() {
    m_pPImpl->substitutionRulesMap.clear();
}

void BehaviorTreeFactory::AddSubstitutionRule(std::string_view filter, SubstitutionRule rule) {
    m_pPImpl->substitutionRulesMap[std::string(filter)] = rule;
}

void BehaviorTreeFactory::LoadSubstitutionRuleFromJSON(const std::string &rJsonText) {
    auto const json = nlohmann::json::parse(rJsonText);

    std::unordered_map<std::string, TestNodeConfig> configsMap;

    auto testConfigs = json.at("TestNodeConfigs");
    for(auto const &[rName, rTestConfig]: testConfigs.items()) {
        auto &rConfig = configsMap[rName];

        auto returnStatus = rTestConfig.at("returnStatus").get<std::string>();
        rConfig.returnStatus = ConvertFromString<NodeStatus>(returnStatus);
        if(rTestConfig.contains("asyncDelay")) {
            rConfig.asyncDelay = std::chrono::milliseconds(rTestConfig["asyncDelay"].get<int>());
        }
        if(rTestConfig.contains("postScript")) {
            rConfig.postScript = rTestConfig["postScript"].get<std::string>();
        }
        if(rTestConfig.contains("successScript")) {
            rConfig.successScript = rTestConfig["successScript"].get<std::string>();
        }
        if(rTestConfig.contains("failureScript")) {
            rConfig.failureScript = rTestConfig["failureScript"].get<std::string>();
        }
    }

    auto substitutions = json.at("SubstitutionRules");
    for(auto const &[rNodeName, rTest]: substitutions.items()) {
        auto testName = rTest.get<std::string>();
        auto iter = configsMap.find(testName);
        if(iter == configsMap.end()) {
            AddSubstitutionRule(rNodeName, testName);
        } else {
            AddSubstitutionRule(rNodeName, iter->second);
        }
    }
}

const std::unordered_map<std::string, BehaviorTreeFactory::SubstitutionRule> &BehaviorTreeFactory::SubstitutionRules() const {
    return m_pPImpl->substitutionRulesMap;
}

Tree &Tree::operator=(Tree &&rOther) {
    m_subtreeVec = std::move(rOther.m_subtreeVec);
    m_manifestsMap = std::move(rOther.m_manifestsMap);
    m_wakeUp = rOther.m_wakeUp;
    return *this;
}

Tree::Tree() {}

Tree::Tree(Tree &&rOther) {
    (*this) = std::move(rOther);
}

void Tree::Initialize() {
    m_wakeUp = std::make_shared<WakeUpSignal>();
    for(auto &rSubtree: m_subtreeVec) {
        for(auto &rNode: rSubtree->nodeVec) {
            rNode->SetWakeUpInstance(m_wakeUp);
        }
    }
}

void Tree::HaltTree() {
    if(!GetRootNode()) {
        return;
    }
    // the halt should propagate to all the node if the nodes
    // have been implemented correctly
    GetRootNode()->HaltNode();

    //but, just in case.... this should be no-op
    auto visitor = [](behaviortree::TreeNode *pNode) {
        pNode->HaltNode();
    };
    behaviortree::ApplyRecursiveVisitor(GetRootNode(), visitor);

    GetRootNode()->ResetNodeStatus();
}

TreeNode *Tree::GetRootNode() const {
    if(m_subtreeVec.empty()) {
        return nullptr;
    }
    auto &rSubtreeNodeVec = m_subtreeVec.front()->nodeVec;
    return rSubtreeNodeVec.empty() ? nullptr : rSubtreeNodeVec.front().get();
}

void Tree::Sleep(std::chrono::system_clock::duration timeout) {
    m_wakeUp->WaitFor(std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
}

Tree::~Tree() {
    HaltTree();
}

NodeStatus Tree::TickExactlyOnce() {
    return TickRoot(ExactlyOnce, std::chrono::milliseconds(0));
}

NodeStatus Tree::TickOnce() {
    return TickRoot(OnceUnlessWokenUp, std::chrono::milliseconds(0));
}

NodeStatus Tree::TickWhileRunning(std::chrono::milliseconds sleepTime) {
    return TickRoot(WhileRunning, sleepTime);
}

Blackboard::Ptr Tree::RootBlackboard() {
    if(m_subtreeVec.size() > 0) {
        return m_subtreeVec.front()->pBlackboard;
    }
    return {};
}

void Tree::ApplyVisitor(const std::function<void(const TreeNode *)> &rVisitor) {
    behaviortree::ApplyRecursiveVisitor(static_cast<const TreeNode *>(GetRootNode()), rVisitor);
}

void Tree::ApplyVisitor(const std::function<void(TreeNode *)> &rVisitor) {
    behaviortree::ApplyRecursiveVisitor(static_cast<TreeNode *>(GetRootNode()), rVisitor);
}

uint16_t Tree::GetUid() {
    auto uid = ++m_uidCounter;
    return uid;
}

NodeStatus Tree::TickRoot(TickOption opt, std::chrono::milliseconds sleepTime) {
    NodeStatus nodeStatus = NodeStatus::Idle;

    if(!m_wakeUp) {
        Initialize();
    }

    if(!GetRootNode()) {
        throw RuntimeError("Empty Tree");
    }

    while(nodeStatus == NodeStatus::Idle || (opt == TickOption::WhileRunning && nodeStatus == NodeStatus::Running)) {
        nodeStatus = GetRootNode()->ExecuteTick();

        // Inner loop. The previous tick might have triggered the wake-up
        // in this case, unless TickOption::EXACTLY_ONCE, we tick again
        while(opt != TickOption::ExactlyOnce && nodeStatus == NodeStatus::Running && m_wakeUp->WaitFor(std::chrono::milliseconds(0))) {
            nodeStatus = GetRootNode()->ExecuteTick();
        }

        if(IsNodeStatusCompleted(nodeStatus)) {
            GetRootNode()->ResetNodeStatus();
        }
        if(nodeStatus == NodeStatus::Running && sleepTime.count() > 0) {
            Sleep(std::chrono::milliseconds(sleepTime));
        }
    }

    return nodeStatus;
}

void BlackboardRestore(const std::vector<Blackboard::Ptr> &rBackup, Tree &rTree) {
    assert(rBackup.size() == rTree.m_subtreeVec.size());
    for(size_t i = 0; i < rTree.m_subtreeVec.size(); i++) {
        rBackup[i]->CloneInto(*(rTree.m_subtreeVec[i]->pBlackboard));
    }
}

std::vector<Blackboard::Ptr> BlackboardBackup(const Tree &rTree) {
    std::vector<Blackboard::Ptr> blackboardVec;
    blackboardVec.reserve(rTree.m_subtreeVec.size());
    for(const auto &pSubtree: rTree.m_subtreeVec) {
        blackboardVec.push_back(behaviortree::Blackboard::Create());
        pSubtree->pBlackboard->CloneInto(*blackboardVec.back());
    }
    return blackboardVec;
}

nlohmann::json ExportTreeToJson(const behaviortree::Tree &rTree) {
    nlohmann::json jsonTree;
    for(const auto &pSubtree: rTree.m_subtreeVec) {
        nlohmann::json jsonSubtree;
        auto subName = pSubtree->instanceName;
        if(subName.empty()) {
            subName = pSubtree->treeId;
        }
        jsonTree[subName] = ExportBlackboardToJson(*pSubtree->pBlackboard);
    }
    return jsonTree;
}

void ImportTreeFromJson(const nlohmann::json &rJson, behaviortree::Tree &rTree) {
    if(rJson.size() != rTree.m_subtreeVec.size()) {
        throw std::runtime_error("Number of blackboards don't match:");
    }

    size_t index = 0;
    for(auto &[refKey, refArray]: rJson.items()) {
        auto &refSubtree = rTree.m_subtreeVec.at(index++);
        ImportBlackboardFromJson(refArray, *refSubtree->pBlackboard);
    }
}

}// namespace behaviortree
