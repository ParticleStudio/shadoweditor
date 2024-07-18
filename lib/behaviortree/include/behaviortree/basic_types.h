#ifndef BEHAVIORTREE_BASIC_TYPES_H
#define BEHAVIORTREE_BASIC_TYPES_H

#include <chrono>
#include <functional>
#include <iostream>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "behaviortree/contrib/expected.hpp"
#include "behaviortree/define.h"
#include "behaviortree/exceptions.h"
#include "behaviortree/util/safe_any.hpp"

namespace behaviortree {
/// Enumerates the possible types of nodes
enum class NodeType {
    Undefined = 0,
    Action = 1,
    Condition = 2,
    Control = 3,
    Decorator = 4,
    Subtree = 5
};

/// Enumerates the states every node can be in after execution during a particular
/// time step.
/// IMPORTANT: Your custom nodes should NEVER return Idle.
enum class NodeStatus {
    Idle = 0,
    Running = 1,
    Success = 2,
    Failure = 3,
    Skipped = 4,
};

inline bool IsStatusActive(const NodeStatus &rNodeStatus) {
    return rNodeStatus != NodeStatus::Idle && rNodeStatus != NodeStatus::Skipped;
}

inline bool IsNodeStatusCompleted(const NodeStatus &rNodeStatus) {
    return rNodeStatus == NodeStatus::Success || rNodeStatus == NodeStatus::Failure;
}

enum class PortDirection {
    Input = 0,
    Output = 1,
    Inout = 2
};

bool StartWith(std::string_view str, std::string_view prefix);

bool StartWith(std::string_view str, char prefix);

// vector of key/value pairs
using KeyValueVector = std::vector<std::pair<std::string, std::string>>;

/** Usage: given a function/method like this:
 *
 *     Expected<double> getAnswer();
 *
 * User code can check result and error message like this:
 *
 *     auto res = getAnswer();
 *     if( res )
 *     {
 *         std::cout << "answer was: " << res.value() << std::endl;
 *     }
 *     else{
 *         std::cerr << "failed to Get the answer: " << res.error() << std::endl;
 *     }
 *
 * */
template<typename T>
using Expected = nonstd::expected<T, std::string>;

struct AnyTypeAllowed {};

/**
 * @brief convertFromJSON will parse a json string and use JsonExporter
 * to Convert its content to a given Type. It will work only if
 * the Type was previously registered. May throw if it fails.
 *
 * @param json_text a valid JSON string
 * @param type you must specify the typeid()
 * @return the object, wrapped in Any.
 */
[[nodiscard]] Any ConvertFromJSON(std::string_view jsonText, std::type_index type);

/// Same as the non template version, but with automatic casting
template<typename T>
[[nodiscard]] inline T ConvertFromJSON(std::string_view str) {
    return ConvertFromJSON(str, typeid(T)).Cast<T>();
}

/**
 * convertFromString is used to Convert a string into a custom Type.
 *
 * This function is invoked under the hood by TreeNode::getInput(), but only when the
 * input port contains a string.
 *
 * If you have a custom Type, you need to implement the corresponding
 * template specialization.
 *
 * If the string starts with the prefix "json:", it will
 * fall back to convertFromJSON()
 */
template<typename T>
[[nodiscard]] inline T ConvertFromString(std::string_view str) {
    // if string starts with "json:{", try to parse it as json
    if(StartWith(str, "json:")) {
        str.remove_prefix(5);
        return ConvertFromJSON<T>(str);
    }

    auto typeName = behaviortree::Demangle(typeid(T));

    std::cerr << "You (maybe indirectly) called "
                 "behaviortree::ConvertFromString() for Type ["
              << typeName
              << "], but I can't find the template specialization.\n"
              << std::endl;

    throw LogicError(std::string("You didn't implement the template specialization of convertFromString for this Type: ") + typeName);
}

template<>
[[nodiscard]] std::string ConvertFromString<std::string>(std::string_view str);

template<>
[[nodiscard]] const char *ConvertFromString<const char *>(std::string_view str);

template<>
[[nodiscard]] int8_t ConvertFromString<int8_t>(std::string_view str);

template<>
[[nodiscard]] int16_t ConvertFromString<int16_t>(std::string_view str);

template<>
[[nodiscard]] int32_t ConvertFromString<int32_t>(std::string_view str);

template<>
[[nodiscard]] int64_t ConvertFromString<int64_t>(std::string_view str);

template<>
[[nodiscard]] uint8_t ConvertFromString<uint8_t>(std::string_view str);

template<>
[[nodiscard]] uint16_t ConvertFromString<uint16_t>(std::string_view str);

template<>
[[nodiscard]] uint32_t ConvertFromString<uint32_t>(std::string_view str);

template<>
[[nodiscard]] uint64_t ConvertFromString<uint64_t>(std::string_view str);

template<>
[[nodiscard]] float ConvertFromString<float>(std::string_view str);

template<>
[[nodiscard]] double ConvertFromString<double>(std::string_view str);

// Integer numbers separated by the character ";"
template<>
[[nodiscard]] std::vector<int> ConvertFromString<std::vector<int>>(std::string_view str);

// Real numbers separated by the character ";"
template<>
[[nodiscard]] std::vector<double> ConvertFromString<std::vector<double>>(std::string_view str);

// Strings separated by the character ";"
template<>
[[nodiscard]] std::vector<std::string> ConvertFromString<std::vector<std::string>>(std::string_view str);

// This recognizes either 0/1, true/false, TRUE/FALSE
template<>
[[nodiscard]] bool ConvertFromString<bool>(std::string_view str);

// Names with all capital letters
template<>
[[nodiscard]] NodeStatus ConvertFromString<NodeStatus>(std::string_view str);

// Names with all capital letters
template<>
[[nodiscard]] NodeType ConvertFromString<NodeType>(std::string_view str);

template<>
[[nodiscard]] PortDirection ConvertFromString<PortDirection>(std::string_view str);

using StringConverter = std::function<Any(std::string_view)>;

using StringConvertersMap = std::unordered_map<const std::type_info *, StringConverter>;

// helper function
template<typename T>
[[nodiscard]] inline StringConverter GetAnyFromStringFunctor() {
    if constexpr(std::is_constructible_v<std::string_view, T>) {
        return [](std::string_view str) {
            return Any(str);
        };
    } else if constexpr(std::is_same_v<behaviortree::AnyTypeAllowed, T> || std::is_enum_v<T>) {
        return {};
    } else {
        return [](std::string_view str) {
            return Any(ConvertFromString<T>(str));
        };
    }
}

template<>
[[nodiscard]] inline StringConverter GetAnyFromStringFunctor<void>() {
    return {};
}

//------------------------------------------------------------------

template<typename T>
constexpr bool IsConvertibleToString() {
    return std::is_convertible_v<T, std::string> || std::is_convertible_v<T, std::string_view>;
}

Expected<std::string> ToJsonString(const Any &rValue);

/**
 * @brief toStr is the reverse operation of convertFromString.
 *
 * If T is a custom Type and there is no template specialization,
 * it will try to fall back to toJsonString()
 */
template<typename T>
[[nodiscard]] std::string ToStr(const T &rValue) {
    if constexpr(IsConvertibleToString<T>()) {
        return static_cast<std::string>(rValue);
    } else if constexpr(!std::is_arithmetic_v<T>) {
        if(auto str = ToJsonString(Any(rValue))) {
            return *str;
        }

        throw LogicError(StrCat("Function BT::toStr<T>() not specialized for Type [", behaviortree::Demangle(typeid(T)), "]"));
    } else {
        return std::to_string(rValue);
    }
}

template<>
[[nodiscard]] std::string ToStr<bool>(const bool &rValue);

template<>
[[nodiscard]] std::string ToStr<std::string>(const std::string &rValue);

template<>
[[nodiscard]] std::string ToStr<behaviortree::NodeStatus>(const behaviortree::NodeStatus &rNodeStatus);

/**
 * @brief toStr converts NodeStatus to string. Optionally colored.
 */
[[nodiscard]] std::string ToStr(behaviortree::NodeStatus nodeStatus, bool colored);

std::ostream &operator<<(std::ostream &rOS, const behaviortree::NodeStatus &rNodeStatus);

template<>
[[nodiscard]] std::string ToStr<behaviortree::NodeType>(const behaviortree::NodeType &rNodeType);

std::ostream &operator<<(std::ostream &rOS, const behaviortree::NodeType &rNodeType);

template<>
[[nodiscard]] std::string ToStr<behaviortree::PortDirection>(const behaviortree::PortDirection &rDirection);

std::ostream &operator<<(std::ostream &rOS, const behaviortree::PortDirection &rPortDirection);

// Small utility, unless you want to use <boost/algorithm/string.hpp>
[[nodiscard]] std::vector<std::string_view> SplitString(const std::string_view &rStrToSplit, char delimeter);

template<typename Predicate>
using EnableIf = typename std::enable_if<Predicate::value>::type *;

template<typename Predicate>
using EnableIfNot = typename std::enable_if<!Predicate::value>::type *;

/** Usage: given a function/method like:
 *
 *     Result DoSomething();
 *
 * User code can check result and error message like this:
 *
 *     auto res = DoSomething();
 *     if( res )
 *     {
 *         std::cout << "DoSomething() done " << std::endl;
 *     }
 *     else{
 *         std::cerr << "DoSomething() failed with message: " << res.error() << std::endl;
 *     }
 *
 * */
using Result = Expected<std::monostate>;

struct Timestamp {
    // Number being incremented every time a new value is written
    uint64_t seq{0};
    // Last update time. Nanoseconds since epoch
    std::chrono::nanoseconds time{std::chrono::nanoseconds(0)};
};

[[nodiscard]] bool IsAllowedPortName(std::string_view str);

class TypeInfo {
 public:
    template<typename T>
    static TypeInfo Create() {
        return TypeInfo{typeid(T), GetAnyFromStringFunctor<T>()};
    }

    TypeInfo(): m_typeInfo(typeid(AnyTypeAllowed)), m_typeStr("AnyTypeAllowed") {
    }

    TypeInfo(std::type_index typeInfo, StringConverter conv): m_typeInfo(typeInfo),
                                                              m_converter(conv),
                                                              m_typeStr(behaviortree::Demangle(typeInfo)) {
    }

    [[nodiscard]] const std::type_index &Type() const;

    [[nodiscard]] const std::string &TypeName() const;

    [[nodiscard]] Any ParseString(const char *pStr) const;

    [[nodiscard]] Any ParseString(const std::string &rStr) const;

    template<typename T>
    [[nodiscard]] Any ParseString(const T &) const {
        // avoid compilation errors
        return {};
    }

    [[nodiscard]] bool IsStronglyTyped() const {
        return m_typeInfo != typeid(AnyTypeAllowed) && m_typeInfo != typeid(behaviortree::Any);
    }

    [[nodiscard]] const StringConverter &Converter() const {
        return m_converter;
    }

 private:
    std::type_index m_typeInfo;
    StringConverter m_converter;
    std::string m_typeStr;
};

class PortInfo: public TypeInfo {
 public:
    PortInfo(PortDirection direction = PortDirection::Inout): TypeInfo(), m_direction(direction) {
    }

    PortInfo(PortDirection direction, std::type_index typeInfo, StringConverter conv): TypeInfo(typeInfo, conv), m_direction(direction) {
    }

    [[nodiscard]] PortDirection Direction() const;

    void SetDescription(std::string_view description);

    template<typename T>
    void SetDefaultValue(const T &refDefaultValue) {
        m_defaultValue = Any(refDefaultValue);
        try {
            m_defaultValueStr = behaviortree::ToStr(refDefaultValue);
        } catch(LogicError &) {
        }
    }

    [[nodiscard]] const std::string &Description() const;

    [[nodiscard]] const Any &DefaultValue() const;

    [[nodiscard]] const std::string &DefaultValueString() const;

 private:
    PortDirection m_direction;
    std::string m_description;
    Any m_defaultValue;
    std::string m_defaultValueStr;
};

template<typename T = AnyTypeAllowed>
[[nodiscard]] std::pair<std::string, PortInfo> CreatePort(PortDirection direction, std::string_view name, std::string_view description = {}) {
    auto sName = static_cast<std::string>(name);
    if(!IsAllowedPortName(sName)) {
        throw RuntimeError(
                "The name of a port must not be `name` or `ID` "
                "and must start with an alphabetic character. "
                "Underscore is reserved."
        );
    }

    std::pair<std::string, PortInfo> out;

    if(std::is_same<T, void>::value) {
        out = {sName, PortInfo(direction)};
    } else {
        out = {sName, PortInfo(direction, typeid(T), GetAnyFromStringFunctor<T>())};
    }
    if(!description.empty()) {
        out.second.SetDescription(description);
    }
    return out;
}

//----------
/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INPUT, ...)
 *
 *  @param name the name of the port
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo> InputPort(std::string_view name, std::string_view description = {}) {
    return CreatePort<T>(PortDirection::Input, name, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::OUTPUT,...)
 *
 *  @param name the name of the port
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo> OutputPort(std::string_view name, std::string_view description = {}) {
    return CreatePort<T>(PortDirection::Output, name, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INOUT,...)
 *
 *  @param name the name of the port
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo> BidirectionalPort(std::string_view name, std::string_view description = {}) {
    return CreatePort<T>(PortDirection::Inout, name, description);
}
//----------

namespace details {

template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo> PortWithDefault(PortDirection direction, std::string_view name, const DefaultT &rDefaultValue, std::string_view description) {
    static_assert(IsConvertibleToString<DefaultT>() || std::is_convertible_v<T, DefaultT> || std::is_constructible_v<T, DefaultT>, "The default value must be either the same of the port or string");

    auto out = CreatePort<T>(direction, name, description);

    if constexpr(std::is_constructible_v<T, DefaultT>) {
        out.second.SetDefaultValue(T(rDefaultValue));
    } else if constexpr(IsConvertibleToString<DefaultT>()) {
        out.second.SetDefaultValue(std::string(rDefaultValue));
    } else {
        out.second.SetDefaultValue(rDefaultValue);
    }
    return out;
}

}// end namespace details

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INPUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default value of the port, either Type T of BlackboardKey
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo> InputPort(std::string_view name, const DefaultT &rDefaultValue, std::string_view description) {
    return details::PortWithDefault<T, DefaultT>(PortDirection::Input, name, rDefaultValue, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INOUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default value of the port, either Type T of BlackboardKey
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo> BidirectionalPort(std::string_view name, const DefaultT &rDefaultValue, std::string_view description) {
    return details::PortWithDefault<T, DefaultT>(PortDirection::Inout, name, rDefaultValue, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::OUTPUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default blackboard entry where the output is written
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo> OutputPort(std::string_view name, std::string_view defaultValue, std::string_view description) {
    if(defaultValue.empty() || defaultValue.front() != '{' || defaultValue.back() != '}') {
        throw LogicError("Output port can only refer to blackboard entries, i.e. use the syntax '{port_name}'");
    }
    auto out = CreatePort<T>(PortDirection::Output, name, description);
    out.second.SetDefaultValue(defaultValue);
    return out;
}

//----------

using PortMap = std::unordered_map<std::string, PortInfo>;

template<typename T, typename = void>
struct HasStaticMethodProvidedPorts: std::false_type {};

template<typename T>
struct HasStaticMethodProvidedPorts<T, typename std::enable_if<std::is_same<decltype(T::ProvidedPorts()), PortMap>::value>::type>: std::true_type {};

template<typename T, typename = void>
struct has_static_method_metadata: std::false_type {};

template<typename T>
struct has_static_method_metadata<T, typename std::enable_if<std::is_same<decltype(T::metadata()), KeyValueVector>::value>::type>: std::true_type {};

template<typename T>
[[nodiscard]] inline PortMap GetProvidedPorts(EnableIf<HasStaticMethodProvidedPorts<T>> = nullptr) {
    return T::ProvidedPorts();
}

template<typename T>
[[nodiscard]] inline PortMap GetProvidedPorts(EnableIfNot<HasStaticMethodProvidedPorts<T>> = nullptr) {
    return {};
}

using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::high_resolution_clock::duration;

}// namespace behaviortree

#endif//BEHAVIORTREE_BASIC_TYPES_H
