#include "behaviortree/factory.h"

#include <filesystem>

#include "behaviortree/contrib/json.hpp"
#include "behaviortree/util/shared_library.h"
#include "behaviortree/util/wildcards.hpp"
#include "behaviortree/xml_parsing.h"

namespace behaviortree {
bool WildcardMatch(std::string const& refStr, StringView filter) {
    return wildcards::match(refStr, filter);
}

struct BehaviorTreeFactory::PImpl {
    std::unordered_map<std::string, NodeBuilder> buildersMap;
    std::unordered_map<std::string, TreeNodeManifest> manifestsMap;
    std::set<std::string> builtinIdsSet;
    std::unordered_map<std::string, Any> behaviortreeDefinitionsMap;
    std::shared_ptr<std::unordered_map<std::string, int>> ptrScriptingEnums;
    std::shared_ptr<behaviortree::Parser> ptrParser;
    std::unordered_map<std::string, SubstitutionRule> substitutionRulesMap;
};

BehaviorTreeFactory::BehaviorTreeFactory(): m_P(new PImpl) {
    m_P->ptrParser = std::make_shared<XMLParser>(*this);
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

    RegisterNodeType<SubTreeNode>("SubTree");

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
    RegisterNodeType<EntryUpdatedDecorator>("SkipUnlessUpdated", NodeStatus::SKIPPED);
    RegisterNodeType<EntryUpdatedDecorator>("WaitValueUpdate", NodeStatus::RUNNING);

    for(const auto& refIt: m_P->buildersMap) {
        m_P->builtinIdsSet.insert(refIt.first);
    }

    m_P->ptrScriptingEnums = std::make_shared<std::unordered_map<std::string, int>>();
}

BehaviorTreeFactory::BehaviorTreeFactory(BehaviorTreeFactory&& refOther) noexcept {
    this->m_P = std::move(refOther.m_P);
}

BehaviorTreeFactory& BehaviorTreeFactory::operator=(BehaviorTreeFactory&& refOther) noexcept {
    this->m_P = std::move(refOther.m_P);
    return *this;
}

BehaviorTreeFactory::~BehaviorTreeFactory() {}

bool BehaviorTreeFactory::UnregisterBuilder(const std::string& refId) {
    if(BuiltinNodes().count(refId)) {
        throw LogicError("You can not remove the builtin registration ID [", refId, "]");
    }
    auto it = m_P->buildersMap.find(refId);
    if(it == m_P->buildersMap.end()) {
        return false;
    }
    m_P->buildersMap.erase(refId);
    m_P->manifestsMap.erase(refId);
    return true;
}

void BehaviorTreeFactory::RegisterBuilder(const TreeNodeManifest& refManifest, const NodeBuilder& refBuilder) {
    auto it = m_P->buildersMap.find(refManifest.registrationId);
    if(it != m_P->buildersMap.end()) {
        throw BehaviorTreeException("Id [", refManifest.registrationId, "] already registered");
    }

    m_P->buildersMap.insert({refManifest.registrationId, refBuilder});
    m_P->manifestsMap.insert({refManifest.registrationId, refManifest});
}

void BehaviorTreeFactory::RegisterSimpleCondition(
        const std::string& refId, const SimpleConditionNode::TickFunctor& refTickFunctor,
        PortsList ports
) {
    NodeBuilder builder = [refTickFunctor, refId](const std::string& refName, const NodeConfig& refConfig) {
        return std::make_unique<SimpleConditionNode>(refName, refTickFunctor, refConfig);
    };

    TreeNodeManifest manifest = {NodeType::CONDITION, refId, std::move(ports), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterSimpleAction(
        const std::string& refId, const SimpleActionNode::TickFunctor& refTickFunctor,
        PortsList ports
) {
    NodeBuilder builder = [refTickFunctor, refId](const std::string& refName, const NodeConfig& refConfig) {
        return std::make_unique<SimpleActionNode>(refName, refTickFunctor, refConfig);
    };

    TreeNodeManifest manifest = {NodeType::ACTION, refId, std::move(ports), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterSimpleDecorator(
        const std::string& refId, const SimpleDecoratorNode::TickFunctor& refTickFunctor,
        PortsList ports
) {
    NodeBuilder builder = [refTickFunctor, refId](const std::string& refName, const NodeConfig& refConfig) {
        return std::make_unique<SimpleDecoratorNode>(refName, refTickFunctor, refConfig);
    };

    TreeNodeManifest manifest = {NodeType::DECORATOR, refId, std::move(ports), {}};
    RegisterBuilder(manifest, builder);
}

void BehaviorTreeFactory::RegisterFromPlugin(const std::string& refFilePath) {
    behaviortree::SharedLibrary loader;
    loader.Load(refFilePath);
    typedef void (*Func)(BehaviorTreeFactory&);

    if(loader.HasSymbol(PLUGIN_SYMBOL)) {
        Func func = (Func)loader.GetSymbol(PLUGIN_SYMBOL);
        func(*this);
    } else {
        std::cout << "ERROR loading library [" << refFilePath << "]: can't find symbol ["
                  << PLUGIN_SYMBOL << "]" << std::endl;
    }
}

#ifdef USING_ROS
#ifdef _WIN32
const char os_pathsep(';');// NOLINT
#else
const char os_pathsep(':');// NOLINT
#endif

// This function is a copy from the one in class_loader_imp.hpp in ROS pluginlib
// package, licensed under BSD.
// https://github.com/ros/pluginlib
std::vector<std::string> getCatkinLibraryPaths() {
    std::vector<std::string> lib_paths;
    const char* env = std::getenv("CMAKE_PREFIX_PATH");
    if(env) {
        const std::string env_catkin_prefix_paths(env);
        std::vector<BT::StringView> catkin_prefix_paths =
                splitString(env_catkin_prefix_paths, os_pathsep);
        for(BT::StringView catkin_prefix_path: catkin_prefix_paths) {
            std::filesystem::path path(static_cast<std::string>(catkin_prefix_path));
            std::filesystem::path lib("lib");
            lib_paths.push_back((path / lib).string());
        }
    }
    return lib_paths;
}

void BehaviorTreeFactory::registerFromROSPlugins() {
    std::vector<std::string> plugins;
    ros::package::getPlugins("behaviortree", "bt_lib_plugin", plugins, true);
    std::vector<std::string> catkin_lib_paths = getCatkinLibraryPaths();

    for(const auto& plugin: plugins) {
        auto filename = std::filesystem::path(plugin + BT::SharedLibrary::suffix());
        for(const auto& lib_path: catkin_lib_paths) {
            const auto full_path = std::filesystem::path(lib_path) / filename;
            if(std::filesystem::exists(full_path)) {
                std::cout << "Registering ROS plugins from " << full_path.string() << std::endl;
                registerFromPlugin(full_path.string());
                break;
            }
        }
    }
}
#endif

void BehaviorTreeFactory::RegisterBehaviorTreeFromFile(
        const std::filesystem::path& refFileName
) {
    m_P->ptrParser->LoadFromFile(refFileName);
}

void BehaviorTreeFactory::RegisterBehaviorTreeFromText(const std::string& xml_text) {
    m_P->ptrParser->LoadFromText(xml_text);
}

std::vector<std::string> BehaviorTreeFactory::RegisteredBehaviorTrees() const {
    return m_P->ptrParser->RegisteredBehaviorTrees();
}

void BehaviorTreeFactory::ClearRegisteredBehaviorTrees() {
    m_P->ptrParser->ClearInternalState();
}

std::unique_ptr<TreeNode> BehaviorTreeFactory::InstantiateTreeNode(
        const std::string& refName, const std::string& refId, const NodeConfig& refConfig
) const {
    auto idNotFound = [this, refId] {
        std::cerr << refId << " not included in this list:" << std::endl;
        for(const auto& refBuilderIt: m_P->buildersMap) {
            std::cerr << refBuilderIt.first << std::endl;
        }
        throw RuntimeError("BehaviorTreeFactory: ID [", refId, "] not registered");
    };

    auto manifestIt = m_P->manifestsMap.find(refId);
    if(manifestIt == m_P->manifestsMap.end()) {
        idNotFound();
    }

    std::unique_ptr<TreeNode> node;

    bool substituted = false;
    for(const auto& [filter, rule]: m_P->substitutionRulesMap) {
        if(filter == refName || filter == refId || wildcards::match(refConfig.path, filter)) {
            // first case: the rule is simply a string with the name of the
            // node to create instead
            if(const auto ptrsSbstitutedId = std::get_if<std::string>(&rule)) {
                auto ptrBuilderIt = m_P->buildersMap.find(*ptrsSbstitutedId);
                if(ptrBuilderIt != m_P->buildersMap.end()) {
                    auto& refBuilder = ptrBuilderIt->second;
                    node = refBuilder(refName, refConfig);
                } else {
                    throw RuntimeError("Substituted Node ID [", *ptrsSbstitutedId, "] not found");
                }
                substituted = true;
                break;
            } else if(const auto ptrTestConfig = std::get_if<TestNodeConfig>(&rule)) {
                // second case, the varian is a TestNodeConfig
                auto ptrTestNode = new TestNode(refName, refConfig, *ptrTestConfig);
                node.reset(ptrTestNode);
                substituted = true;
                break;
            }
        }
    }

    // No substitution rule applied: default behavior
    if(!substituted) {
        auto it_builder = m_P->buildersMap.find(refId);
        if(it_builder == m_P->buildersMap.end()) {
            idNotFound();
        }
        auto& builder = it_builder->second;
        node = builder(refName, refConfig);
    }

    node->SetRegistrationId(refId);
    node->GetConfig().ptrEnums = m_P->ptrScriptingEnums;

    auto assignConditions = [](auto& conditions, auto& executors) {
        for(const auto& [refCondId, refScript]: conditions) {
            if(auto executor = ParseScript(refScript)) {
                executors[size_t(refCondId)] = executor.value();
            } else {
                throw LogicError("Error in the script \"", refScript, "\"\n", executor.error());
            }
        }
    };
    assignConditions(refConfig.preConditions, node->PreConditionsScripts());
    assignConditions(refConfig.postConditions, node->PostConditionsScripts());

    return node;
}

const std::unordered_map<std::string, NodeBuilder>& BehaviorTreeFactory::Builders() const {
    return m_P->buildersMap;
}

const std::unordered_map<std::string, TreeNodeManifest>&
BehaviorTreeFactory::Manifests() const {
    return m_P->manifestsMap;
}

const std::set<std::string>& BehaviorTreeFactory::BuiltinNodes() const {
    return m_P->builtinIdsSet;
}

Tree BehaviorTreeFactory::CreateTreeFromText(const std::string& refText, Blackboard::Ptr blackboard) {
    if(!m_P->ptrParser->RegisteredBehaviorTrees().empty()) {
        std::cout << "WARNING: You executed BehaviorTreeFactory::CreateTreeFromText "
                     "after registerBehaviorTreeFrom[File/Text].\n"
                     "This is NOT, probably, what you want to do.\n"
                     "You should probably use BehaviorTreeFactory::CreateTree, instead"
                  << std::endl;
    }
    XMLParser parser(*this);
    parser.LoadFromText(refText);
    auto tree = parser.InstantiateTree(blackboard);
    tree.m_ManifestsMap = this->Manifests();
    return tree;
}

Tree BehaviorTreeFactory::CreateTreeFromFile(const std::filesystem::path& refFilePath, Blackboard::Ptr blackboard) {
    if(!m_P->ptrParser->RegisteredBehaviorTrees().empty()) {
        std::cout << "WARNING: You executed BehaviorTreeFactory::CreateTreeFromFile "
                     "after registerBehaviorTreeFrom[File/Text].\n"
                     "This is NOT, probably, what you want to do.\n"
                     "You should probably use BehaviorTreeFactory::CreateTree, instead"
                  << std::endl;
    }

    XMLParser parser(*this);
    parser.LoadFromFile(refFilePath);
    auto tree = parser.InstantiateTree(blackboard);
    tree.m_ManifestsMap = this->Manifests();
    return tree;
}

Tree BehaviorTreeFactory::CreateTree(const std::string& refTreeName, Blackboard::Ptr blackboard) {
    auto tree = m_P->ptrParser->InstantiateTree(blackboard, refTreeName);
    tree.m_ManifestsMap = this->Manifests();
    return tree;
}

void BehaviorTreeFactory::AddMetadataToManifest(const std::string& refNodeId, const KeyValueVector& refMetadata) {
    auto it = m_P->manifestsMap.find(refNodeId);
    if(it == m_P->manifestsMap.end()) {
        throw std::runtime_error("AddMetadataToManifest: wrong ID");
    }
    it->second.metadata = refMetadata;
}

void BehaviorTreeFactory::RegisterScriptingEnum(StringView name, int value) {
    const auto str = std::string(name);
    auto it = m_P->ptrScriptingEnums->find(str);
    if(it == m_P->ptrScriptingEnums->end()) {
        m_P->ptrScriptingEnums->insert({str, value});
    } else {
        if(it->second != value) {
            throw LogicError(
                    StrCat("Registering the enum [", name, "] twice with different values, first ",
                           std::to_string(it->second), " and later ", std::to_string(value))
            );
        }
    }
}

void BehaviorTreeFactory::ClearSubstitutionRules() {
    m_P->substitutionRulesMap.clear();
}

void BehaviorTreeFactory::AddSubstitutionRule(StringView filter, SubstitutionRule rule) {
    m_P->substitutionRulesMap[std::string(filter)] = rule;
}

void BehaviorTreeFactory::LoadSubstitutionRuleFromJSON(const std::string& refJsonText) {
    auto const json = nlohmann::json::parse(refJsonText);

    std::unordered_map<std::string, TestNodeConfig> configsMap;

    auto testConfigs = json.at("TestNodeConfigs");
    for(auto const& [refName, refTestConfig]: testConfigs.items()) {
        auto& refConfig = configsMap[refName];

        auto returnStatus = refTestConfig.at("returnStatus").get<std::string>();
        refConfig.returnStatus = ConvertFromString<NodeStatus>(returnStatus);
        if(refTestConfig.contains("asyncDelay")) {
            refConfig.asyncDelay =
                    std::chrono::milliseconds(refTestConfig["asyncDelay"].get<int>());
        }
        if(refTestConfig.contains("postScript")) {
            refConfig.postScript = refTestConfig["postScript"].get<std::string>();
        }
        if(refTestConfig.contains("successScript")) {
            refConfig.successScript = refTestConfig["successScript"].get<std::string>();
        }
        if(refTestConfig.contains("failureScript")) {
            refConfig.failureScript = refTestConfig["failureScript"].get<std::string>();
        }
    }

    auto substitutions = json.at("SubstitutionRules");
    for(auto const& [refNodeName, refTest]: substitutions.items()) {
        auto testName = refTest.get<std::string>();
        auto it = configsMap.find(testName);
        if(it == configsMap.end()) {
            AddSubstitutionRule(refNodeName, testName);
        } else {
            AddSubstitutionRule(refNodeName, it->second);
        }
    }
}

const std::unordered_map<std::string, BehaviorTreeFactory::SubstitutionRule>&
BehaviorTreeFactory::SubstitutionRules() const {
    return m_P->substitutionRulesMap;
}

Tree& Tree::operator=(Tree&& refOther) {
    ptrSubtrees = std::move(refOther.ptrSubtrees);
    m_ManifestsMap = std::move(refOther.m_ManifestsMap);
    m_WakeUp = refOther.m_WakeUp;
    return *this;
}

Tree::Tree() {}

Tree::Tree(Tree&& refOther) {
    (*this) = std::move(refOther);
}

void Tree::Initialize() {
    m_WakeUp = std::make_shared<WakeUpSignal>();
    for(auto& refSubtree: ptrSubtrees) {
        for(auto& refNode: refSubtree->ptrNodes) {
            refNode->SetWakeUpInstance(m_WakeUp);
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
    auto visitor = [](behaviortree::TreeNode* node) { node->HaltNode(); };
    behaviortree::ApplyRecursiveVisitor(GetRootNode(), visitor);

    GetRootNode()->ResetNodeStatus();
}

TreeNode* Tree::GetRootNode() const {
    if(ptrSubtrees.empty()) {
        return nullptr;
    }
    auto& refSubtreeNodes = ptrSubtrees.front()->ptrNodes;
    return refSubtreeNodes.empty() ? nullptr : refSubtreeNodes.front().get();
}

void Tree::Sleep(std::chrono::system_clock::duration timeout) {
    m_WakeUp->WaitFor(std::chrono::duration_cast<std::chrono::milliseconds>(timeout));
}

Tree::~Tree() {
    HaltTree();
}

NodeStatus Tree::TickExactlyOnce() {
    return TickRoot(EXACTLY_ONCE, std::chrono::milliseconds(0));
}

NodeStatus Tree::TickOnce() {
    return TickRoot(ONCE_UNLESS_WOKEN_UP, std::chrono::milliseconds(0));
}

NodeStatus Tree::TickWhileRunning(std::chrono::milliseconds sleepTime) {
    return TickRoot(WHILE_RUNNING, sleepTime);
}

Blackboard::Ptr Tree::RootBlackboard() {
    if(ptrSubtrees.size() > 0) {
        return ptrSubtrees.front()->ptrBlackboard;
    }
    return {};
}

void Tree::ApplyVisitor(const std::function<void(const TreeNode*)>& refVisitor) {
    behaviortree::ApplyRecursiveVisitor(static_cast<const TreeNode*>(GetRootNode()), refVisitor);
}

void Tree::ApplyVisitor(const std::function<void(TreeNode*)>& refVisitor) {
    behaviortree::ApplyRecursiveVisitor(static_cast<TreeNode*>(GetRootNode()), refVisitor);
}

uint16_t Tree::GetUID() {
    auto uid = ++m_uidCounter;
    return uid;
}

NodeStatus Tree::TickRoot(TickOption opt, std::chrono::milliseconds sleepTime) {
    NodeStatus nodeStatus = NodeStatus::IDLE;

    if(!m_WakeUp) {
        Initialize();
    }

    if(!GetRootNode()) {
        throw RuntimeError("Empty Tree");
    }

    while(nodeStatus == NodeStatus::IDLE ||
          (opt == TickOption::WHILE_RUNNING && nodeStatus == NodeStatus::RUNNING)) {
        nodeStatus = GetRootNode()->ExecuteTick();

        // Inner loop. The previous tick might have triggered the wake-up
        // in this case, unless TickOption::EXACTLY_ONCE, we tick again
        while(opt != TickOption::EXACTLY_ONCE && nodeStatus == NodeStatus::RUNNING &&
              m_WakeUp->WaitFor(std::chrono::milliseconds(0))) {
            nodeStatus = GetRootNode()->ExecuteTick();
        }

        if(IsStatusCompleted(nodeStatus)) {
            GetRootNode()->ResetNodeStatus();
        }
        if(nodeStatus == NodeStatus::RUNNING && sleepTime.count() > 0) {
            Sleep(std::chrono::milliseconds(sleepTime));
        }
    }

    return nodeStatus;
}

void BlackboardRestore(const std::vector<Blackboard::Ptr>& refBackup, Tree& refTree) {
    assert(refBackup.size() == refTree.ptrSubtrees.size());
    for(size_t i = 0; i < refTree.ptrSubtrees.size(); i++) {
        refBackup[i]->CloneInto(*(refTree.ptrSubtrees[i]->ptrBlackboard));
    }
}

std::vector<Blackboard::Ptr> BlackboardBackup(const Tree& refTree) {
    std::vector<Blackboard::Ptr> bb;
    bb.reserve(refTree.ptrSubtrees.size());
    for(const auto& refSub: refTree.ptrSubtrees) {
        bb.push_back(behaviortree::Blackboard::Create());
        refSub->ptrBlackboard->CloneInto(*bb.back());
    }
    return bb;
}

nlohmann::json ExportTreeToJSON(const Tree& refTree) {
    nlohmann::json out;
    for(const auto& refSubtree: refTree.ptrSubtrees) {
        nlohmann::json jsonSub;
        auto subName = refSubtree->instanceName;
        if(subName.empty()) {
            subName = refSubtree->treeId;
        }
        out[subName] = ExportBlackboardToJSON(*refSubtree->ptrBlackboard);
    }
    return out;
}

void ImportTreeFromJSON(const nlohmann::json& refJson, Tree& refTree) {
    if(refJson.size() != refTree.ptrSubtrees.size()) {
        throw std::runtime_error("Number of blackboards don't match:");
    }

    size_t index = 0;
    for(auto& [refKey, refArray]: refJson.items()) {
        auto& refSubtree = refTree.ptrSubtrees.at(index++);
        ImportBlackboardFromJSON(refArray, *refSubtree->ptrBlackboard);
    }
}

}// namespace behaviortree
