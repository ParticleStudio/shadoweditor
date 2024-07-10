#ifndef BEHAVIORTREE_BT_FACTORY_H
#define BEHAVIORTREE_BT_FACTORY_H

#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "behaviortree/behaviortree.h"
#include "behaviortree/contrib/magic_enum.hpp"

namespace behaviortree {
/// The term "Builder" refers to the Builder Pattern (https://en.wikipedia.org/wiki/Builder_pattern)
using NodeBuilder =
        std::function<std::unique_ptr<TreeNode>(const std::string&, const NodeConfig&)>;

template<typename T, typename... Args>
inline NodeBuilder CreateBuilder(Args... args) {
    return [=](const std::string& refName, const NodeConfig& refConfig) {
        return TreeNode::Instantiate<T, Args...>(refName, refConfig, args...);
    };
}

template<typename T>
inline TreeNodeManifest CreateManifest(const std::string& refId,
                                       PortsList portlist = GetProvidedPorts<T>()) {
    if constexpr(has_static_method_metadata<T>::value) {
        return {GetType<T>(), refId, portlist, T::metadata()};
    }
    return {GetType<T>(), refId, portlist, {}};
}

#ifdef BT_PLUGIN_EXPORT

#if defined(_WIN32)
#define BTCPP_EXPORT extern "C" __declspec(dllexport)
#else
// Unix-like OSes
#define BTCPP_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#else
#define BTCPP_EXPORT static
#endif
/* Use this macro to automatically register one or more custom Nodes
* into a factory. For instance:
*
*   BT_REGISTER_NODES(factory)
*   {
*     factory.registerNodeType<MoveBaseAction>("MoveBase");
*   }
*
* IMPORTANT: this function MUST be declared in a cpp file, NOT a header file.
* You must add the definition [BT_PLUGIN_EXPORT] in CMakeLists.txt using:
*
*   target_compile_definitions(my_plugin_target PRIVATE  BT_PLUGIN_EXPORT )

* See examples in sample_nodes directory.
*/

#define BT_REGISTER_NODES(factory) \
    BTCPP_EXPORT void BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory& factory)

constexpr const char* PLUGIN_SYMBOL{"BT_RegisterNodesFromPlugin"};

bool WildcardMatch(const std::string& refStr, StringView filter);

/**
 * @brief Struct used to store a tree.
 * If this object goes out of scope, the tree is destroyed.
 */
class Tree {
 public:
    // a tree can contain multiple subtree.
    struct Subtree {
        using Ptr = std::shared_ptr<Subtree>;
        std::vector<TreeNode::Ptr> ptrNodes;
        Blackboard::Ptr ptrBlackboard;
        std::string instanceName;
        std::string treeId;
    };

    std::vector<Subtree::Ptr> ptrSubtrees;
    std::unordered_map<std::string, TreeNodeManifest> m_ManifestsMap;

    Tree();

    Tree(const Tree&) = delete;
    Tree& operator=(const Tree&) = delete;

    Tree(Tree&& refOther);
    Tree& operator=(Tree&& refOther);

    void Initialize();

    void HaltTree();

    [[nodiscard]] TreeNode* GetRootNode() const;

    /// Sleep for a certain amount of time.
    /// This Sleep could be interrupted by the method
    /// TreeNode::emitWakeUpSignal()
    void Sleep(std::chrono::system_clock::duration timeout);

    ~Tree();

    /// Tick the root of the tree once, even if a node invoked
    /// emitWakeUpSignal()
    NodeStatus TickExactlyOnce();

    /**
   * @brief by default, TickOnce() sends a single tick, BUT
   * as long as there is at least one node of the tree
   * invoking TreeNode::emitWakeUpSignal(), it will be ticked again.
   */
    NodeStatus TickOnce();

    /// Call TickOnce until the status is different from RUNNING.
    /// Note that between one tick and the following one,
    /// a Tree::Sleep() is used
    NodeStatus
    TickWhileRunning(std::chrono::milliseconds sleepTime = std::chrono::milliseconds(10));

    [[nodiscard]] Blackboard::Ptr RootBlackboard();

    //Call the visitor for each node of the tree.
    void ApplyVisitor(const std::function<void(const TreeNode*)>& refVisitor);

    //Call the visitor for each node of the tree.
    void ApplyVisitor(const std::function<void(TreeNode*)>& refVisitor);

    [[nodiscard]] uint16_t GetUID();

    /// Get a list of nodes which GetFullPath() match a wildcard filter and
    /// a given path. Example:
    ///
    /// move_nodes = tree.GetNodesByPath<MoveBaseNode>("move_*");
    ///
    template<typename NodeType = behaviortree::TreeNode>
    [[nodiscard]] std::vector<const TreeNode*>
    GetNodesByPath(StringView wildcardFilter) const {
        std::vector<const TreeNode*> nodesVec;
        for(auto const& refSubtree: ptrSubtrees) {
            for(auto const& refNode: refSubtree->ptrNodes) {
                if(auto nodeRecast = dynamic_cast<const NodeType*>(refNode.get())) {
                    if(WildcardMatch(refNode->GetFullPath(), wildcardFilter)) {
                        nodesVec.push_back(refNode.get());
                    }
                }
            }
        }
        return nodesVec;
    }

 private:
    std::shared_ptr<WakeUpSignal> m_WakeUp;

    enum TickOption {
        EXACTLY_ONCE,
        ONCE_UNLESS_WOKEN_UP,
        WHILE_RUNNING
    };

    NodeStatus TickRoot(TickOption opt, std::chrono::milliseconds sleepTime);

    uint16_t m_uidCounter{0};
};

class Parser;

/**
 * @brief The BehaviorTreeFactory is used to create instances of a
 * TreeNode at Run-time.
 *
 * Some node types are "builtin", whilst other are used defined and need
 * to be registered using a unique ID.
 */
class BehaviorTreeFactory {
 public:
    BehaviorTreeFactory();
    ~BehaviorTreeFactory();

    BehaviorTreeFactory(const BehaviorTreeFactory& refOther) = delete;
    BehaviorTreeFactory& operator=(const BehaviorTreeFactory& refOther) = delete;

    BehaviorTreeFactory(BehaviorTreeFactory&& refOther) noexcept;
    BehaviorTreeFactory& operator=(BehaviorTreeFactory&& refOther) noexcept;

    /// Remove a registered ID.
    bool UnregisterBuilder(const std::string& refId);

    /** The most generic way to register a NodeBuilder.
    *
    * Throws if you try to register twice a builder with the same
    * registration_ID.
    */
    void RegisterBuilder(const TreeNodeManifest& refManifest, const NodeBuilder& refBuilder);

    template<typename T>
    void RegisterBuilder(const std::string& refId, const NodeBuilder& refBuilder) {
        auto manifest = CreateManifest<T>(refId);
        RegisterBuilder(manifest, refBuilder);
    }

    /**
    * @brief RegisterSimpleAction help you register nodes of Type SimpleActionNode.
    *
    * @param refName            registration ID
    * @param refConfig  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
    void RegisterSimpleAction(const std::string& refName,
                              const SimpleActionNode::TickFunctor& refConfig,
                              PortsList ports = {});
    /**
    * @brief RegisterSimpleCondition help you register nodes of Type SimpleConditionNode.
    *
    * @param refName            registration ID
    * @param refConfig  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
    void RegisterSimpleCondition(const std::string& refName,
                                 const SimpleConditionNode::TickFunctor& refConfig,
                                 PortsList ports = {});
    /**
    * @brief RegisterSimpleDecorator help you register nodes of Type SimpleDecoratorNode.
    *
    * @param refName            registration ID
    * @param refConfig  the callback to be invoked in the tick() method.
    * @param ports         if your SimpleNode requires ports, provide the list here.
    *
    * */
    void RegisterSimpleDecorator(const std::string& refName,
                                 const SimpleDecoratorNode::TickFunctor& refConfig,
                                 PortsList ports = {});

    /**
     * @brief RegisterFromPlugin load a shared library and execute the function BT_REGISTER_NODES (see macro).
     *
     * @param refFilePath path of the file
     */
    void RegisterFromPlugin(const std::string& refFilePath);

    /**
     * @brief RegisterBehaviorTreeFromFile.
     * Load the definition of an entire behavior tree, but don't instantiate it.
     * You can instantiate it later with:
     *
     *    BehaviorTreeFactory::CreateTree(tree_id)
     *
     * where "tree_id" come from the XML attribute <BehaviorTree ID="tree_id">
     *
     */
    void RegisterBehaviorTreeFromFile(const std::filesystem::path& refFileName);

    /// Same of RegisterBehaviorTreeFromFile, but passing the XML text,
    /// instead of the filename.
    void RegisterBehaviorTreeFromText(const std::string& refXmlText);

    /// Returns the ID of the trees registered either with
    /// RegisterBehaviorTreeFromFile or RegisterBehaviorTreeFromText.
    [[nodiscard]] std::vector<std::string> RegisteredBehaviorTrees() const;

    /**
   * @brief Clear previously-registered behavior trees.
   */
    void ClearRegisteredBehaviorTrees();

    /**
     * @brief InstantiateTreeNode creates an instance of a previously registered TreeNode.
     *
     * @param refName     name of this particular instance
     * @param refId       ID used when it was registered
     * @param refConfig   configuration that is passed to the constructor of the TreeNode.
     * @return         new node.
     */
    [[nodiscard]] std::unique_ptr<TreeNode> InstantiateTreeNode(
            const std::string& refName, const std::string& refId, const NodeConfig& refConfig) const;

    /** RegisterNodeType where you explicitly pass the list of ports.
   *  Doesn't require the implementation of static method ProvidedPorts()
  */
    template<typename T, typename... ExtraArgs>
    void RegisterNodeType(const std::string& refId, const PortsList& refPorts, ExtraArgs... args) {
        static_assert(std::is_base_of<ActionNodeBase, T>::value ||
                              std::is_base_of<ControlNode, T>::value ||
                              std::is_base_of<DecoratorNode, T>::value ||
                              std::is_base_of<ConditionNode, T>::value,
                      "[registerNode]: accepts only classed derived from either "
                      "ActionNodeBase, "
                      "DecoratorNode, ControlNode or ConditionNode");

        constexpr bool default_constructable =
                std::is_constructible<T, const std::string&>::value;
        constexpr bool param_constructable =
                std::is_constructible<T, const std::string&, const NodeConfig&,
                                      ExtraArgs...>::value;

        // clang-format off
    static_assert(!std::is_abstract<T>::value,
                  "[registerNode]: Some methods are pure virtual. "
                  "Did you override the methods tick() and halt()?");

    static_assert(default_constructable || param_constructable,
       "[registerNode]: the registered class must have at least one of these two constructors:\n"
       "  (const std::string&, const NodeConfig&) or (const std::string&)\n"
       "Check also if the constructor is public!)");
        // clang-format on

        RegisterBuilder(CreateManifest<T>(refId, refPorts), CreateBuilder<T>(args...));
    }

    /** RegisterNodeType is the method to use to register your custom TreeNode.
  *
  *  It accepts only classed derived from either ActionNodeBase, DecoratorNode,
  *  ControlNode or ConditionNode.
  */
    template<typename T, typename... ExtraArgs>
    void RegisterNodeType(const std::string& refId, ExtraArgs... args) {
        if constexpr(std::is_abstract_v<T>) {
            // check first if the given class is abstract
            static_assert(!std::is_abstract_v<T>,
                          "The Node Type can't be abstract. "
                          "Did you forget to implement an abstract "
                          "method in the derived class?");
        } else {
            constexpr bool paramConstructable =
                    std::is_constructible<T, const std::string&, const NodeConfig&,
                                          ExtraArgs...>::value;
            constexpr bool hasStaticPortsList = HasStaticMethodProvidedPorts<T>::value;

            static_assert(!(paramConstructable && !hasStaticPortsList),
                          "[registerNode]: you MUST implement the static method:\n"
                          "  PortsList ProvidedPorts();\n");

            static_assert(!(hasStaticPortsList && !paramConstructable),
                          "[registerNode]: since you have a static method ProvidedPorts(),\n"
                          "you MUST Add a constructor with signature:\n"
                          "(const std::string&, const NodeConfig&)\n");
        }

        RegisterNodeType<T>(refId, GetProvidedPorts<T>(), args...);
    }

    /// All the Builders. Made available mostly for debug purposes.
    [[nodiscard]] const std::unordered_map<std::string, NodeBuilder>& Builders() const;

    /// Manifests of all the registered TreeNodes.
    [[nodiscard]] const std::unordered_map<std::string, TreeNodeManifest>&
    Manifests() const;

    /// List of builtin IDs.
    [[nodiscard]] const std::set<std::string>& BuiltinNodes() const;

    /**
   * @brief CreateTreeFromText will parse the XML directly from string.
   * The XML needs to contain either a single <BehaviorTree> or specify
   * the attribute [main_tree_to_execute].
   *
   * Consider using instead RegisterBehaviorTreeFromText() and CreateTree().
   *
   * @param refText        string containing the XML
   * @param blackboard  blackboard of the root tree
   * @return the newly created tree
   */
    [[nodiscard]] Tree CreateTreeFromText(
            const std::string& refText, Blackboard::Ptr blackboard = Blackboard::Create());

    /**
   * @brief CreateTreeFromFile will parse the XML from a given file.
   * The XML needs to contain either a single <BehaviorTree> or specify
   * the attribute [main_tree_to_execute].
   *
   * Consider using instead RegisterBehaviorTreeFromFile() and CreateTree().
   *
   * @param refFilePath   location of the file to load
   * @param blackboard  blackboard of the root tree
   * @return the newly created tree
   */
    [[nodiscard]] Tree
    CreateTreeFromFile(const std::filesystem::path& refFilePath,
                       Blackboard::Ptr blackboard = Blackboard::Create());

    [[nodiscard]] Tree CreateTree(const std::string& refTreeName,
                                  Blackboard::Ptr blackboard = Blackboard::Create());

    /// Add metadata to a specific manifest. This metadata will be added
    /// to <TreeNodesModel> with the function WriteTreeNodesModelXML()
    void AddMetadataToManifest(const std::string& refNodeId, const KeyValueVector& refMetadata);

    /**
   * @brief Add an Enum to the scripting language.
   * For instance if you do:
   *
   * RegisterScriptingEnum("THE_ANSWER", 42),
   *
   * You may Type this in your scripts:
   *
   * <Script code="myport:=THE_ANSWER" />
   *
   * @param name    string representation of the enum
   * @param value   its value.
   */
    void RegisterScriptingEnum(StringView name, int value);

    /**
   * @brief RegisterScriptingEnums is syntactic sugar
   * to automatically register multiple enums. We use
   * https://github.com/Neargye/magic_enum.
   *
   * Please refer to https://github.com/Neargye/magic_enum/blob/master/doc/limitations.md
   * for limitations.
   */
    template<typename EnumType>
    void RegisterScriptingEnums() {
        constexpr auto entries = magic_enum::enum_entries<EnumType>();
        for(const auto& refIt: entries) {
            RegisterScriptingEnum(refIt.second, static_cast<int>(refIt.first));
        }
    }

    void ClearSubstitutionRules();

    using SubstitutionRule = std::variant<std::string, TestNodeConfig>;

    /**
   * @brief AddSubstitutionRule replace a node with another one when the tree is
   * created.
   * If the rule ia a string, we will use a diferent node Type (already registered)
   * instead.
   * If the rule is a TestNodeConfig, a test node with that configuration will be created instead.
   *
   * @param filter   filter used to select the node to sobstitute. The node path is used.
   *                 You may use wildcard matching.
   * @param rule     pass either a string or a TestNodeConfig
   */
    void AddSubstitutionRule(StringView filter, SubstitutionRule rule);

    /**
   * @brief LoadSubstitutionRuleFromJSON will parse a JSON file to
   * create a set of substitution rules. See Tutorial 11
   * for an example of the syntax.
   *
   * @param refJsonText  the JSON file as text (BOT the path of the file)
   */
    void LoadSubstitutionRuleFromJSON(const std::string& refJsonText);

    /**
   * @brief SubstitutionRules return the current substitution rules.
   */
    [[nodiscard]] const std::unordered_map<std::string, SubstitutionRule>&
    SubstitutionRules() const;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_P;
};

/**
 * @brief BlackboardClone make a copy of the content of the
 * blackboard
 * @param refSrc   source
 * @param refDst   destination
 */
void BlackboardClone(const Blackboard& refSrc, Blackboard& refDst);

/**
 * @brief BlackboardBackup uses Blackboard::cloneInto to backup
 * all the blackboards of the tree
 *
 * @param refTree source
 * @return destination (the backup)
 */
std::vector<Blackboard::Ptr> BlackboardBackup(const behaviortree::Tree& refTree);

/**
 * @brief BlackboardRestore uses Blackboard::cloneInto to restore
 * all the blackboards of the tree
 *
 * @param refBackup a vectror of blackboards
 * @param refTree the destination
 */
void BlackboardRestore(const std::vector<Blackboard::Ptr>& refBackup, behaviortree::Tree& refTree);

/**
 * @brief ExportTreeToJSON it calls ExportBlackboardToJSON
 * for all the blackboards in the tree
 */
nlohmann::json ExportTreeToJSON(const behaviortree::Tree& refTree);

/**
 * @brief ImportTreeFromJSON it calls ImportBlackboardFromJSON
 * for all the blackboards in the tree
 */
void ImportTreeFromJSON(const nlohmann::json& refJson, behaviortree::Tree& refTree);

}// namespace behaviortree

#endif// BEHAVIORTREE_BT_FACTORY_H
