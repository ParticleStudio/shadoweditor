#ifndef BEHAVIORTREE_TREE_NODE_H
#define BEHAVIORTREE_TREE_NODE_H

#include <exception>
#include <map>
#include <utility>

#include "behaviortree/basic_types.h"
#include "behaviortree/blackboard.h"
#include "behaviortree/scripting/script_parser.hpp"
#include "behaviortree/util/signal.h"
#include "behaviortree/util/strcat.hpp"
#include "behaviortree/util/wakeup_signal.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4127)
#endif

namespace behaviortree {
/// This information is used mostly by the XMLParser.
struct TreeNodeManifest {
    NodeType type;
    std::string registrationId;
    PortsList ports;
    KeyValueVector metadata;
};

using PortsRemapping = std::unordered_map<std::string, std::string>;

enum class PreCond {
    // order of the enums also tell us the execution order
    FAILURE_IF = 0,
    SUCCESS_IF,
    SKIP_IF,
    WHILE_TRUE,
    COUNT
};

enum class PostCond {
    // order of the enums also tell us the execution order
    ON_HALTED = 0,
    ON_FAILURE,
    ON_SUCCESS,
    ALWAYS,
    COUNT
};

template<>
[[nodiscard]] std::string ToStr<behaviortree::PostCond>(const behaviortree::PostCond& refPre);

template<>
[[nodiscard]] std::string ToStr<behaviortree::PreCond>(const behaviortree::PreCond& refPre);

using ScriptingEnumsRegistry = std::unordered_map<std::string, int>;

struct NodeConfig {
    NodeConfig() {}

    // Pointer to the blackboard used by this node
    Blackboard::Ptr ptrBlackboard;
    // List of enums available for scripting
    std::shared_ptr<ScriptingEnumsRegistry> ptrEnums;
    // input ports
    PortsRemapping inputPortsMap;
    // output ports
    PortsRemapping outputPortsMap;

    const TreeNodeManifest* ptrManifest{nullptr};

    // Numberic unique identifier
    uint16_t uid{0};
    // Unique human-readable name, that encapsulate the subtree
    // hierarchy, for instance, given 2 nested trees, it should be:
    //
    //   main_tree/nested_tree/my_action
    std::string path;

    std::map<PreCond, std::string> preConditions;
    std::map<PostCond, std::string> postConditions;
};

// back compatibility
using NodeConfiguration = NodeConfig;

template<typename T>
inline constexpr bool HasNodeNameCtor() {
    return std::is_constructible<T, const std::string&>::value;
}

template<typename T, typename... ExtraArgs>
inline constexpr bool HasNodeFullCtor() {
    return std::is_constructible<T, const std::string&, const NodeConfig&,
                                 ExtraArgs...>::value;
}

/// Abstract base class for Behavior Tree Nodes
class TreeNode {
 public:
    typedef std::shared_ptr<TreeNode> Ptr;

    /**
     * @brief TreeNode main constructor.
     *
     * @param name     name of the instance, not the Type.
     * @param config   information about input/output ports. See NodeConfig
     *
     * Note: If your custom node has ports, the derived class must implement:
     *
     *     static PortsList providedPorts();
     */
    TreeNode(std::string name, NodeConfig config);

    TreeNode(const TreeNode& refOther) = delete;
    TreeNode& operator=(const TreeNode& refOther) = delete;

    TreeNode(TreeNode&& refOther) noexcept;
    TreeNode& operator=(TreeNode&& refOther) noexcept;

    virtual ~TreeNode();

    /// The method that should be used to invoke tick() and setStatus();
    virtual behaviortree::NodeStatus ExecuteTick();

    void HaltNode();

    [[nodiscard]] bool IsHalted() const;

    [[nodiscard]] NodeStatus GetNodeStatus() const;

    /// GetNodeName of the instance, not the Type
    [[nodiscard]] const std::string& GetNodeName() const;

    /// Blocking function that will Sleep until the setStatus() is called with
    /// either RUNNING, FAILURE or SUCCESS.
    [[nodiscard]] behaviortree::NodeStatus WaitValidStatus();

    virtual NodeType Type() const = 0;

    using StatusChangeSignal = Signal<TimePoint, const TreeNode&, NodeStatus, NodeStatus>;
    using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
    using StatusChangeCallback = StatusChangeSignal::CallableFunction;

    using PreTickCallback = std::function<NodeStatus(TreeNode&)>;
    using PostTickCallback = std::function<NodeStatus(TreeNode&, NodeStatus)>;
    using TickMonitorCallback =
            std::function<void(TreeNode&, NodeStatus, std::chrono::microseconds)>;

    /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * When StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed automatically.
     *
     * @param callback The callback to be execute when status change.
     *
     * @return the subscriber handle.
     */
    [[nodiscard]] StatusChangeSubscriber
    SubscribeToStatusChange(StatusChangeCallback callback);

    /** This method attaches to the TreeNode a callback with signature:
     *
     *     NodeStatus callback(TreeNode& node)
     *
     * This callback is executed BEFORE the tick() and, if it returns SUCCESS or FAILURE,
     * the actual tick() will NOT be executed and this result will be returned instead.
     *
     * This is useful to inject a "dummy" implementation of the TreeNode at Run-time
     */
    void SetPreTickFunction(PreTickCallback callback);

    /**
    * This method attaches to the TreeNode a callback with signature:
    *
    *     NodeStatus myCallback(TreeNode& node, NodeStatus status)
    *
    * This callback is executed AFTER the tick() and, if it returns SUCCESS or FAILURE,
    * the value returned by the actual tick() is overriden with this one.
    */
    void SetPostTickFunction(PostTickCallback callback);

    /**
   * This method attaches to the TreeNode a callback with signature:
   *
   *     void myCallback(TreeNode& node, NodeStatus status, std::chrono::microseconds duration)
   *
   * This callback is executed AFTER the tick() and will inform the user about its status and
   * the execution time. Works only if the tick was not substituted by a pre-condition.
   */
    void SetTickMonitorCallback(TickMonitorCallback callback);

    /// The unique identifier of this instance of treeNode.
    /// It is assigneld by the factory
    [[nodiscard]] uint16_t GetUID() const;

    /// Human readable identifier, that includes the hierarchy of Subtrees
    /// See tutorial 10 as an example.
    [[nodiscard]] const std::string& GetFullPath() const;

    /// registrationName is the ID used by BehaviorTreeFactory to create an instance.
    [[nodiscard]] const std::string& GetRegistrAtionName() const;

    /// Configuration passed at construction time. Can never change after the
    /// creation of the TreeNode instance.
    [[nodiscard]] const NodeConfig& GetConfig() const;

    /** Read an input port, which, in practice, is an entry in the blackboard.
   * If the blackboard contains a std::string and T is not a string,
   * convertFromString<T>() is used automatically to parse the text.
   *
   * @param key   the name of the port.
   * @param destination  reference to the object where the value should be stored
   * @return      false if an error occurs.
   */
    template<typename T>
    Result GetInput(const std::string& refKey, T& refDestination) const;

    /**
   * @brief getInputStamped is similar to getInput(dey, destination),
   * but it returne also the Timestamp object, that can be used to check if
   * a value was updated and when.
   *
   * @param key   the name of the port.
   * @param destination  reference to the object where the value should be stored
   */
    template<typename T>
    [[nodiscard]] Expected<Timestamp> GetInputStamped(const std::string& refKey,
                                                      T& refDestination) const;

    /** Same as bool getInput(const std::string& key, T& destination)
   * but using optional.
   *
   * @param key   the name of the port.
   */
    template<typename T>
    [[nodiscard]] Expected<T> GetInput(const std::string& refKey) const {
        T out{};
        auto res = GetInput(refKey, out);
        return (res) ? Expected<T>(out) : nonstd::make_unexpected(res.error());
    }

    /** Same as bool getInputStamped(const std::string& key, T& destination)
   * but return StampedValue<T>
   *
   * @param key   the name of the port.
   */
    template<typename T>
    [[nodiscard]] Expected<StampedValue<T>> GetInputStamped(const std::string& refKey) const {
        StampedValue<T> out;
        if(auto res = GetInputStamped(refKey, out.value)) {
            out.stamp = *res;
            return out;
        } else {
            return nonstd::make_unexpected(res.error());
        }
    }

    /**
   * @brief setOutput modifies the content of an Output port
   * @param key    the name of the port.
   * @param value  new value
   * @return       valid Result, if succesful.
   */
    template<typename T>
    Result SetOutput(const std::string& refKey, const T& refValue);

    /**
   * @brief getLockedPortContent should be used when:
   *
   * - your port contains an object with reference semantic (usually a smart pointer)
   * - you want to modify the object we are pointing to.
   * - you are concerned about thread-safety.
   *
   * For example, if your port has Type std::shared_ptr<Foo>,
   * the code below is NOT thread safe:
   *
   *    auto foo_ptr = getInput<std::shared_ptr<Foo>>("port_name");
   *    // modifying the content of foo_ptr is NOT thread-safe
   *
   * What you must do, instead, to guaranty thread-safety, is:
   *
   *    if(auto any_ref = getLockedPortContent("port_name")) {
   *      Any* any = any_ref.Get();
   *      auto foo_ptr = any->Cast<std::shared_ptr<Foo>>();
   *      // modifying the content of foo_ptr inside this scope IS thread-safe
   *    }
   *
   * It is important to destroy the object AnyPtrLocked, to release the Lock.
   *
   * NOTE: this method doesn't work, if the port contains a static string, instead
   * of a blackboard pointer.
   *
   * @param key  the identifier of the port.
   * @return     Empty AnyPtrLocked if the blackboard entry doesn't exist or the content
   *             of the port was a static string.
   */
    [[nodiscard]] AnyPtrLocked GetLockedPortContent(const std::string& refKey);

    // function provided mostly for debugging purpose to see the raw value
    // in the port (no remapping and no conversion to a type)
    [[nodiscard]] StringView GetRawPortValue(const std::string& refKey) const;

    /// Check a string and return true if it matches the pattern:  {...}
    [[nodiscard]] static bool IsBlackboardPointer(StringView str,
                                                  StringView* ptrStrippedPointer = nullptr);

    [[nodiscard]] static StringView StripBlackboardPointer(StringView str);

    [[nodiscard]] static Expected<StringView> GetRemappedKey(StringView portName,
                                                             StringView remappedPort);

    /// Notify that the tree should be ticked again()
    void EmitWakeUpSignal();

    [[nodiscard]] bool RequiresWakeUp() const;

    /** Used to inject config into a node, even if it doesn't have the proper
     *  constructor
     */
    template<class DerivedT, typename... ExtraArgs>
    static std::unique_ptr<TreeNode> Instantiate(const std::string& refName,
                                                 const NodeConfig& refConfig,
                                                 ExtraArgs... args) {
        static_assert(HasNodeFullCtor<DerivedT, ExtraArgs...>() ||
                      HasNodeNameCtor<DerivedT>());

        if constexpr(HasNodeFullCtor<DerivedT, ExtraArgs...>()) {
            return std::make_unique<DerivedT>(refName, refConfig, args...);
        } else if constexpr(HasNodeNameCtor<DerivedT>()) {
            auto ptrNode = new DerivedT(refName, args...);
            ptrNode->config() = refConfig;
            return std::unique_ptr<DerivedT>(ptrNode);
        }
    }

 protected:
    friend class BehaviorTreeFactory;
    friend class DecoratorNode;
    friend class ControlNode;
    friend class Tree;

    [[nodiscard]] NodeConfig& GetConfig();

    /// Method to be implemented by the user
    virtual behaviortree::NodeStatus Tick() = 0;

    /// Set the status to IDLE
    void ResetNodeStatus();

    // Only BehaviorTreeFactory should call this
    void SetRegistrationID(StringView registrationId);

    void SetWakeUpInstance(std::shared_ptr<WakeUpSignal> ptrInstance);

    void ModifyPortsRemapping(const PortsRemapping& refNewRemapping);

    /**
     * @brief setStatus changes the status of the node.
     * it will throw if you try to change the status to IDLE, because
     * your parent node should do that, not the user!
     */
    void SetNodeStatus(NodeStatus newNodeStatus);

    using PreScripts = std::array<ScriptFunction, size_t(PreCond::COUNT)>;
    using PostScripts = std::array<ScriptFunction, size_t(PostCond::COUNT)>;

    PreScripts& PreConditionsScripts();
    PostScripts& PostConditionsScripts();

    template<typename T>
    T ParseString(const std::string& refStr) const;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_P;

    Expected<NodeStatus> CheckPreConditions();
    void CheckPostConditions(NodeStatus refCond);

    /// The method used to interrupt the execution of a RUNNING node.
    /// Only Async nodes that may return RUNNING should implement it.
    virtual void Halt() = 0;
};

//-------------------------------------------------------

template<typename T>
T TreeNode::ParseString(const std::string& refStr) const {
    if constexpr(std::is_enum_v<T> && !std::is_same_v<T, NodeStatus>) {
        auto ptrIt = GetConfig().ptrEnums->find(refStr);
        // conversion available
        if(ptrIt != GetConfig().ptrEnums->end()) {
            return static_cast<T>(ptrIt->second);
        } else {
            // hopefully str contains a number that can be parsed. May throw
            return static_cast<T>(ConvertFromString<int>(refStr));
        }
    }
    return ConvertFromString<T>(refStr);
}

template<typename T>
inline Expected<Timestamp> TreeNode::GetInputStamped(const std::string& refKey,
                                                     T& refDestination) const {
    std::string portValue;

    auto ptrInputPort = GetConfig().inputPortsMap.find(refKey);
    if(ptrInputPort != GetConfig().inputPortsMap.end()) {
        portValue = ptrInputPort->second;
    } else if(GetConfig().ptrManifest != nullptr) {
        return nonstd::make_unexpected(StrCat("GetInput() of node '", GetFullPath(),
                                              "' failed because the manifest is "
                                              "nullptr (WTF?) and the key: [",
                                              refKey, "] is missing"));
    } else {
        // maybe it is declared with a default value in the manifest
        auto ptrPortManifest = GetConfig().ptrManifest->ports.find(refKey);
        if(ptrPortManifest == GetConfig().ptrManifest->ports.end()) {
            return nonstd::make_unexpected(StrCat("getInput() of node '", GetFullPath(),
                                                  "' failed because the manifest doesn't "
                                                  "contain the key: [",
                                                  refKey, "]"));
        }
        const auto& refPortInfo = ptrPortManifest->second;
        // there is a default value
        if(refPortInfo.DefaultValue().Empty()) {
            return nonstd::make_unexpected(StrCat("getInput() of node '", GetFullPath(),
                                                  "' failed because nor the manifest or the "
                                                  "XML contain the key: [",
                                                  refKey, "]"));
        }
        if(refPortInfo.DefaultValue().IsString()) {
            portValue = refPortInfo.DefaultValue().Cast<std::string>();
        } else {
            refDestination = refPortInfo.DefaultValue().Cast<T>();
            return Timestamp{};
        }
    }

    auto ptrBlackboard = GetRemappedKey(refKey, portValue);
    try {
        // pure string, not a blackboard key
        if(ptrBlackboard == nullptr) {
            try {
                refDestination = parseString<T>(portValue);
            } catch(std::exception& ex) {
                return nonstd::make_unexpected(StrCat("getInput(): ", ex.what()));
            }
            return Timestamp{};
        }
        const auto& refBlackboardKey = ptrBlackboard.value();

        if(GetConfig().ptrBlackboard == nullptr) {
            return nonstd::make_unexpected(
                    "GetInput(): trying to access "
                    "an invalid Blackboard");
        }

        if(auto ptrEntry = GetConfig().ptrBlackboard->GetEntry(std::string(refBlackboardKey))) {
            std::unique_lock lk(ptrEntry->entryMutex);
            auto& refAnyValue = ptrEntry->value;

            // support getInput<Any>()
            if constexpr(std::is_same_v<T, Any>) {
                refDestination = refAnyValue;
                return Timestamp{ptrEntry->sequenceId, ptrEntry->stamp};
            }

            if(!ptrEntry->value.Empty()) {
                if(!std::is_same_v<T, std::string> && refAnyValue.IsString()) {
                    refDestination = parseString<T>(refAnyValue.Cast<std::string>());
                } else {
                    refDestination = refAnyValue.Cast<T>();
                }
                return Timestamp{ptrEntry->sequenceId, ptrEntry->stamp};
            }
        }

        return nonstd::make_unexpected(StrCat(
                "getInput() failed because it was unable to "
                "find the key [",
                refKey, "] remapped to [", refBlackboardKey, "]"));
    } catch(std::exception& err) {
        return nonstd::make_unexpected(err.what());
    }
}

template<typename T>
inline Result TreeNode::GetInput(const std::string& refKey, T& refDestination) const {
    auto res = GetInputStamped(refKey, refDestination);
    if(!res) {
        return nonstd::make_unexpected(res.error());
    }
    return {};
}

template<typename T>
inline Result TreeNode::SetOutput(const std::string& refKey, const T& refValue) {
    if(GetConfig().ptrBlackboard == nullptr) {
        return nonstd::make_unexpected(
                "setOutput() failed: trying to access a "
                "Blackboard(BB) entry, but BB is invalid");
    }

    auto remapIt = GetConfig().outputPortsMap.find(refKey);
    if(remapIt == GetConfig().outputPortsMap.end()) {
        return nonstd::make_unexpected(StrCat(
                "setOutput() failed: "
                "NodeConfig::output_ports "
                "does not contain the key: [",
                refKey, "]"));
    }
    StringView remappedKey = remapIt->second;
    if(remappedKey == "{=}" || remappedKey == "=") {
        GetConfig().ptrBlackboard->Set(static_cast<std::string>(refKey), refValue);
        return {};
    }

    if(!IsBlackboardPointer(remappedKey)) {
        return nonstd::make_unexpected("setOutput requires a blackboard pointer. Use {}");
    }

    if constexpr(std::is_same_v<behaviortree::Any, T>) {
        if(GetConfig().ptrManifest->ports.at(refKey).Type() != typeid(behaviortree::Any)) {
            throw LogicError(
                    "setOutput<Any> is not allowed, unless the port "
                    "was declared using OutputPort<Any>");
        }
    }

    remappedKey = StripBlackboardPointer(remappedKey);
    GetConfig().ptrBlackboard->Set(static_cast<std::string>(remappedKey), refValue);

    return {};
}

// Utility function to fill the list of ports using T::ProvidedPorts();
template<typename T>
inline void AssignDefaultRemapping(NodeConfig& refConfig) {
    for(const auto& refIt: GetProvidedPorts<T>()) {
        const auto& refPortName = refIt.first;
        const auto direction = refIt.second.direction();
        if(direction != PortDirection::OUTPUT) {
            // PortDirection::{INPUT,INOUT}
            refConfig.inputPortsMap[refPortName] = "{=}";
        }
        if(direction != PortDirection::INPUT) {
            // PortDirection::{OUTPUT,INOUT}
            refConfig.outputPortsMap[refPortName] = "{=}";
        }
    }
}

}// namespace behaviortree

#endif// BEHAVIORTREE_TREE_NODE_H
