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
#    pragma warning(disable : 4127)
#endif

namespace behaviortree {
/// This information is used mostly by the XMLParser.
struct TreeNodeManifest {
    NodeType type;
    std::string registrationId;
    PortMap portMap;
    KeyValueVector metadataVec;
};

using PortsRemapping = std::unordered_map<std::string, std::string>;

enum class PreCond {
    // order of the enums also tell us the execution order
    FailureIf = 0,
    SuccessIf = 1,
    SkipIf = 2,
    WhileTrue = 3,
    Count = 4
};

enum class PostCond {
    // order of the enums also tell us the execution order
    OnHalted = 0,
    OnFailure = 1,
    OnSuccess = 2,
    Always = 3,
    Count = 4
};

template<>
[[nodiscard]] std::string ToStr<behaviortree::PostCond>(const behaviortree::PostCond &rPreCond);

template<>
[[nodiscard]] std::string ToStr<behaviortree::PreCond>(const behaviortree::PreCond &rPreCond);

using ScriptingEnumsRegistry = std::unordered_map<std::string, int>;

struct NodeConfig {
    NodeConfig() {}

    // Pointer to the blackboard used by this node
    Blackboard::Ptr pBlackboard;
    // List of enums available for scripting
    std::shared_ptr<ScriptingEnumsRegistry> pEnums;
    // input ports
    PortsRemapping inputPortMap;
    // output ports
    PortsRemapping outputPortMap;

    const TreeNodeManifest *pManifest{nullptr};

    // Numberic unique identifier
    uint16_t uid{0};
    // Unique human-readable name, that encapsulate the subtree
    // hierarchy, for instance, given 2 nested trees, it should be:
    //
    //   main_tree/nested_tree/my_action
    std::string path;

    std::map<PreCond, std::string> preConditionMap;
    std::map<PostCond, std::string> postConditionMap;
};

// back compatibility
using NodeConfiguration = NodeConfig;

template<typename T>
inline constexpr bool HasNodeNameCtor() {
    return std::is_constructible<T, const std::string &>::value;
}

template<typename T, typename... ExtraArgs>
inline constexpr bool HasNodeFullCtor() {
    return std::is_constructible<T, const std::string &, const NodeConfig &, ExtraArgs...>::value;
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
     *     static PortsList ProvidedPorts();
     */
    TreeNode(std::string name, NodeConfig config);

    TreeNode(const TreeNode &rOther) = delete;
    TreeNode &operator=(const TreeNode &rOther) = delete;

    TreeNode(TreeNode &&rOther) noexcept;
    TreeNode &operator=(TreeNode &&rOther) noexcept;

    virtual ~TreeNode();

    /// The method that should be used to invoke tick() and setStatus();
    virtual behaviortree::NodeStatus ExecuteTick();

    void HaltNode();

    [[nodiscard]] bool IsHalted() const;

    [[nodiscard]] NodeStatus GetNodeStatus() const;

    /// GetNodeName of the instance, not the Type
    [[nodiscard]] const std::string &GetNodeName() const;

    /// Blocking function that will Sleep until the setStatus() is called with
    /// either RUNNING, FAILURE or SUCCESS.
    [[nodiscard]] behaviortree::NodeStatus WaitValidStatus();

    virtual NodeType Type() const = 0;

    using StatusChangeSignal = Signal<TimePoint, const TreeNode &, NodeStatus, NodeStatus>;
    using StatusChangeSubscriber = StatusChangeSignal::Subscriber;
    using StatusChangeCallback = StatusChangeSignal::CallableFunction;

    using PreTickCallback = std::function<NodeStatus(TreeNode &)>;
    using PostTickCallback = std::function<NodeStatus(TreeNode &, NodeStatus)>;
    using TickMonitorCallback = std::function<void(TreeNode &, NodeStatus, std::chrono::microseconds)>;

    /**
     * @brief subscribeToStatusChange is used to attach a callback to a status change.
     * When StatusChangeSubscriber goes out of scope (it is a shared_ptr) the callback
     * is unsubscribed automatically.
     *
     * @param callback The callback to be execute when status change.
     *
     * @return the subscriber handle.
     */
    [[nodiscard]] StatusChangeSubscriber SubscribeToStatusChange(
            StatusChangeCallback callback
    );

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
    [[nodiscard]] uint16_t GetUid() const;

    /// Human readable identifier, that includes the hierarchy of Subtrees
    /// See tutorial 10 as an example.
    [[nodiscard]] const std::string &GetFullPath() const;

    /// registrationName is the ID used by BehaviorTreeFactory to create an instance.
    [[nodiscard]] const std::string &GetRegistrAtionName() const;

    /// Configuration passed at construction time. Can never change after the
    /// creation of the TreeNode instance.
    [[nodiscard]] const NodeConfig &GetConfig() const;

    /** Read an input port, which, in practice, is an entry in the blackboard.
   * If the blackboard contains a std::string and T is not a string,
   * convertFromString<T>() is used automatically to parse the text.
   *
   * @param key   the name of the port.
   * @param destination  reference to the object where the value should be stored
   * @return      false if an error occurs.
   */
    template<typename T>
    Result GetInput(const std::string &rKey, T &rDestination) const;

    /**
   * @brief getInputStamped is similar to getInput(dey, destination),
   * but it returne also the Timestamp object, that can be used to check if
   * a value was updated and when.
   *
   * @param key   the name of the port.
   * @param destination  reference to the object where the value should be stored
   */
    template<typename T>
    [[nodiscard]] Expected<Timestamp> GetInputStamped(const std::string &rKey, T &rDestination) const;

    /** Same as bool getInput(const std::string& key, T& destination)
   * but using optional.
   *
   * @param key   the name of the port.
   */
    template<typename T>
    [[nodiscard]] Expected<T> GetInput(const std::string &rKey) const {
        T out{};
        auto res = GetInput(rKey, out);
        return (res) ? Expected<T>(out) : nonstd::make_unexpected(res.error());
    }

    /** Same as bool getInputStamped(const std::string& key, T& destination)
   * but return StampedValue<T>
   *
   * @param key   the name of the port.
   */
    template<typename T>
    [[nodiscard]] Expected<StampedValue<T>> GetInputStamped(const std::string &rKey) const {
        StampedValue<T> out;
        if(auto res = GetInputStamped(rKey, out.value)) {
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
    Result SetOutput(const std::string &rKey, const T &rValue);

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
    [[nodiscard]] AnyPtrLocked GetLockedPortContent(const std::string &rKey);

    // function provided mostly for debugging purpose to see the raw value
    // in the port (no remapping and no conversion to a type)
    [[nodiscard]] std::string_view GetRawPortValue(const std::string &rKey) const;

    /// Check a string and return true if it matches the pattern:  {...}
    [[nodiscard]] static bool IsBlackboardPointer(std::string_view str, std::string_view *pStrippedPointer = nullptr);

    [[nodiscard]] static std::string_view StripBlackboardPointer(std::string_view str);

    [[nodiscard]] static Expected<std::string_view> GetRemappedKey(std::string_view portName, std::string_view remappedPort);

    /// Notify that the tree should be ticked again()
    void EmitWakeUpSignal();

    [[nodiscard]] bool RequiresWakeUp() const;

    /** Used to inject config into a node, even if it doesn't have the proper
     *  constructor
     */
    template<class DerivedT, typename... ExtraArgs>
    static std::unique_ptr<TreeNode> Instantiate(const std::string &rName, const NodeConfig &rConfig, ExtraArgs... args) {
        static_assert(HasNodeFullCtor<DerivedT, ExtraArgs...>() || HasNodeNameCtor<DerivedT>());

        if constexpr(HasNodeFullCtor<DerivedT, ExtraArgs...>()) {
            return std::make_unique<DerivedT>(rName, rConfig, args...);
        } else if constexpr(HasNodeNameCtor<DerivedT>()) {
            auto ptrNode = new DerivedT(rName, args...);
            ptrNode->GetConfig() = rConfig;
            return std::unique_ptr<DerivedT>(ptrNode);
        }
    }

 protected:
    friend class BehaviorTreeFactory;
    friend class DecoratorNode;
    friend class ControlNode;
    friend class Tree;

    [[nodiscard]] NodeConfig &GetConfig();

    /// Method to be implemented by the user
    virtual behaviortree::NodeStatus Tick() = 0;

    /// Set the status to IDLE
    void ResetNodeStatus();

    // Only BehaviorTreeFactory should call this
    void SetRegistrationId(std::string_view registrationId);

    void SetWakeUpInstance(std::shared_ptr<WakeUpSignal> pInstance);

    void ModifyPortsRemapping(const PortsRemapping &rNewRemapping);

    /**
     * @brief setStatus changes the status of the node.
     * it will throw if you try to change the status to IDLE, because
     * your parent node should do that, not the user!
     */
    void SetNodeStatus(NodeStatus newNodeStatus);

    using PreScripts = std::array<ScriptFunction, size_t(PreCond::Count)>;
    using PostScripts = std::array<ScriptFunction, size_t(PostCond::Count)>;

    PreScripts &PreConditionsScripts();
    PostScripts &PostConditionsScripts();

    template<typename T>
    T ParseString(const std::string &rStr) const;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pPImpl;

    Expected<NodeStatus> CheckPreConditions();
    void CheckPostConditions(NodeStatus nodeStatus);

    /// The method used to interrupt the execution of a RUNNING node.
    /// Only Async nodes that may return RUNNING should implement it.
    virtual void Halt() = 0;
};

//-------------------------------------------------------

template<typename T>
T TreeNode::ParseString(const std::string &rStr) const {
    if constexpr(std::is_enum_v<T> && !std::is_same_v<T, NodeStatus>) {
        auto pIt = GetConfig().pEnums->find(rStr);
        // conversion available
        if(pIt != GetConfig().pEnums->end()) {
            return static_cast<T>(pIt->second);
        } else {
            // hopefully str contains a number that can be parsed. May throw
            return static_cast<T>(ConvertFromString<int>(rStr));
        }
    }
    return ConvertFromString<T>(rStr);
}

template<typename T>
inline Expected<Timestamp> TreeNode::GetInputStamped(const std::string &rKey, T &rDestination) const {
    std::string portValue;

    auto ptrInputPort = GetConfig().inputPortMap.find(rKey);
    if(ptrInputPort != GetConfig().inputPortMap.end()) {
        portValue = ptrInputPort->second;
    } else if(GetConfig().pManifest != nullptr) {
        return nonstd::make_unexpected(
                StrCat("GetInput() of node '", GetFullPath(),
                       "' failed because the manifest is "
                       "nullptr (WTF?) and the key: [",
                       rKey, "] is missing")
        );
    } else {
        // maybe it is declared with a default value in the manifest
        auto pPortManifest = GetConfig().pManifest->portMap.find(rKey);
        if(pPortManifest == GetConfig().pManifest->portMap.end()) {
            return nonstd::make_unexpected(
                    StrCat("getInput() of node '", GetFullPath(),
                           "' failed because the manifest doesn't "
                           "contain the key: [",
                           rKey, "]")
            );
        }
        const auto &rPortInfo = pPortManifest->second;
        // there is a default value
        if(rPortInfo.DefaultValue().Empty()) {
            return nonstd::make_unexpected(
                    StrCat("getInput() of node '", GetFullPath(),
                           "' failed because nor the manifest or the "
                           "XML contain the key: [",
                           rKey, "]")
            );
        }
        if(rPortInfo.DefaultValue().IsString()) {
            portValue = rPortInfo.DefaultValue().Cast<std::string>();
        } else {
            rDestination = rPortInfo.DefaultValue().Cast<T>();
            return Timestamp{};
        }
    }

    auto blackboardKey = GetRemappedKey(rKey, portValue);
    try {
        // pure string, not a blackboard key
        if(!blackboardKey) {
            try {
                rDestination = ParseString<T>(portValue);
            } catch(std::exception &ex) {
                return nonstd::make_unexpected(StrCat("getInput(): ", ex.what()));
            }
            return Timestamp{};
        }
        const auto &rBlackboardKey = blackboardKey.value();

        if(GetConfig().pBlackboard == nullptr) {
            return nonstd::make_unexpected(
                    "GetInput(): trying to access "
                    "an invalid Blackboard"
            );
        }

        if(auto pEntry = GetConfig().pBlackboard->GetEntry(std::string(rBlackboardKey))) {
            std::unique_lock lock(pEntry->entryMutex);
            auto &rAnyValue = pEntry->value;

            // support getInput<Any>()
            if constexpr(std::is_same_v<T, Any>) {
                rDestination = rAnyValue;
                return Timestamp{pEntry->sequenceId, pEntry->stamp};
            }

            if(!pEntry->value.Empty()) {
                if(!std::is_same_v<T, std::string> && rAnyValue.IsString()) {
                    rDestination = ParseString<T>(rAnyValue.Cast<std::string>());
                } else {
                    rDestination = rAnyValue.Cast<T>();
                }
                return Timestamp{pEntry->sequenceId, pEntry->stamp};
            }
        }

        return nonstd::make_unexpected(
                StrCat("getInput() failed because it was unable to "
                       "find the key [",
                       rKey, "] remapped to [", rBlackboardKey, "]")
        );
    } catch(std::exception &rError) {
        return nonstd::make_unexpected(rError.what());
    }
}

template<typename T>
inline Result TreeNode::GetInput(const std::string &rKey, T &rDestination) const {
    auto res = GetInputStamped(rKey, rDestination);
    if(!res) {
        return nonstd::make_unexpected(res.error());
    }
    return {};
}

template<typename T>
inline Result TreeNode::SetOutput(const std::string &rKey, const T &rValue) {
    if(GetConfig().pBlackboard == nullptr) {
        return nonstd::make_unexpected(
                "setOutput() failed: trying to access a "
                "Blackboard(BB) entry, but BB is invalid"
        );
    }

    auto remapIt = GetConfig().outputPortMap.find(rKey);
    if(remapIt == GetConfig().outputPortMap.end()) {
        return nonstd::make_unexpected(
                StrCat("setOutput() failed: "
                       "NodeConfig::output_ports "
                       "does not contain the key: [",
                       rKey, "]")
        );
    }
    std::string_view remappedKey = remapIt->second;
    if(remappedKey == "{=}" || remappedKey == "=") {
        GetConfig().pBlackboard->Set(static_cast<std::string>(rKey), rValue);
        return {};
    }

    if(!IsBlackboardPointer(remappedKey)) {
        return nonstd::make_unexpected("setOutput requires a blackboard pointer. Use {}");
    }

    if constexpr(std::is_same_v<behaviortree::Any, T>) {
        if(GetConfig().pManifest->portMap.at(rKey).Type() !=
           typeid(behaviortree::Any)) {
            throw LogicError("setOutput<Any> is not allowed, unless the port was declared using OutputPort<Any>");
        }
    }

    remappedKey = StripBlackboardPointer(remappedKey);
    GetConfig().pBlackboard->Set(static_cast<std::string>(remappedKey), rValue);

    return {};
}

// Utility function to fill the list of ports using T::ProvidedPorts();
template<typename T>
inline void AssignDefaultRemapping(NodeConfig &rConfig) {
    for(const auto &rIt: GetProvidedPorts<T>()) {
        const auto &rPortName = rIt.first;
        const auto direction = rIt.second.direction();
        if(direction != PortDirection::Output) {
            // PortDirection::{INPUT,INOUT}
            rConfig.inputPortMap[rPortName] = "{=}";
        }
        if(direction != PortDirection::Input) {
            // PortDirection::{OUTPUT,INOUT}
            rConfig.outputPortMap[rPortName] = "{=}";
        }
    }
}

}// namespace behaviortree

#endif// BEHAVIORTREE_TREE_NODE_H
