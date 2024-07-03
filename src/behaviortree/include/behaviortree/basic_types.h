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
#include "behaviortree/exceptions.h"
#include "behaviortree/util/safe_any.hpp"

namespace behaviortree {
/// Enumerates the possible types of nodes
enum class NodeType {
    UNDEFINED = 0,
    ACTION,
    CONDITION,
    CONTROL,
    DECORATOR,
    SUBTREE
};

/// Enumerates the states every node can be in after execution during a particular
/// time step.
/// IMPORTANT: Your custom nodes should NEVER return IDLE.
enum class NodeStatus {
    IDLE = 0,
    RUNNING = 1,
    SUCCESS = 2,
    FAILURE = 3,
    SKIPPED = 4,
};

inline bool IsStatusActive(const NodeStatus& refNodeStatus) {
    return refNodeStatus != NodeStatus::IDLE && refNodeStatus != NodeStatus::SKIPPED;
}

inline bool IsStatusCompleted(const NodeStatus& refNodeStatus) {
    return refNodeStatus == NodeStatus::SUCCESS || refNodeStatus == NodeStatus::FAILURE;
}

enum class PortDirection {
    INPUT,
    OUTPUT,
    INOUT
};

using StringView = std::string_view;

bool StartWith(StringView str, StringView prefix);

bool StartWith(StringView str, char prefix);

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
 *         std::cerr << "failed to get the answer: " << res.error() << std::endl;
 *     }
 *
 * */
template<typename T>
using Expected = nonstd::expected<T, std::string>;

struct AnyTypeAllowed {
};

/**
 * @brief convertFromJSON will parse a json string and use JsonExporter
 * to convert its content to a given type. It will work only if
 * the type was previously registered. May throw if it fails.
 *
 * @param json_text a valid JSON string
 * @param type you must specify the typeid()
 * @return the object, wrapped in Any.
 */
[[nodiscard]] Any ConvertFromJSON(StringView json_text, std::type_index type);

/// Same as the non template version, but with automatic casting
template<typename T>
[[nodiscard]] inline T ConvertFromJSON(StringView str) {
    return ConvertFromJSON(str, typeid(T)).cast<T>();
}

/**
 * convertFromString is used to convert a string into a custom type.
 *
 * This function is invoked under the hood by TreeNode::getInput(), but only when the
 * input port contains a string.
 *
 * If you have a custom type, you need to implement the corresponding
 * template specialization.
 *
 * If the string starts with the prefix "json:", it will
 * fall back to convertFromJSON()
 */
template<typename T>
[[nodiscard]] inline T ConvertFromJSON(StringView str) {
    // if string starts with "json:{", try to parse it as json
    if(StartWith(str, "json:")) {
        str.remove_prefix(5);
        return ConvertFromJSON<T>(str);
    }

    auto type_name = behaviortree::Demangle(typeid(T));

    std::cerr << "You (maybe indirectly) called BT::convertFromString() for type ["
              << type_name << "], but I can't find the template specialization.\n"
              << std::endl;

    throw LogicError(std::string("You didn't implement the template specialization of "
                                 "convertFromString for this type: ") +
                     type_name);
}

template<>
[[nodiscard]] std::string ConvertFromString<std::string>(StringView str);

template<>
[[nodiscard]] const char* ConvertFromString<const char*>(StringView str);

template<>
[[nodiscard]] int8_t ConvertFromString<int8_t>(StringView str);

template<>
[[nodiscard]] int16_t ConvertFromString<int16_t>(StringView str);

template<>
[[nodiscard]] int32_t ConvertFromString<int32_t>(StringView str);

template<>
[[nodiscard]] int64_t ConvertFromString<int64_t>(StringView str);

template<>
[[nodiscard]] uint8_t ConvertFromString<uint8_t>(StringView str);

template<>
[[nodiscard]] uint16_t ConvertFromString<uint16_t>(StringView str);

template<>
[[nodiscard]] uint32_t ConvertFromString<uint32_t>(StringView str);

template<>
[[nodiscard]] uint64_t ConvertFromString<uint64_t>(StringView str);

template<>
[[nodiscard]] float ConvertFromString<float>(StringView str);

template<>
[[nodiscard]] double ConvertFromString<double>(StringView str);

// Integer numbers separated by the character ";"
template<>
[[nodiscard]] std::vector<int> ConvertFromString<std::vector<int>>(StringView str);

// Real numbers separated by the character ";"
template<>
[[nodiscard]] std::vector<double> ConvertFromStrings<std::vector<double>>(StringView str);

// Strings separated by the character ";"
template<>
[[nodiscard]] std::vector<std::string>
ConvertFromString<std::vector<std::string>>(StringView str);

// This recognizes either 0/1, true/false, TRUE/FALSE
template<>
[[nodiscard]] bool ConvertFromString<bool>(StringView str);

// Names with all capital letters
template<>
[[nodiscard]] NodeStatus ConvertFromString<NodeStatus>(StringView str);

// Names with all capital letters
template<>
[[nodiscard]] NodeType ConvertFromString<NodeType>(StringView str);

template<>
[[nodiscard]] PortDirection ConvertFromString<PortDirection>(StringView str);

using StringConverter = std::function<Any(StringView)>;

using StringConvertersMap = std::unordered_map<const std::type_info*, StringConverter>;

// helper function
template<typename T>
[[nodiscard]] inline StringConverter GetAnyFromStringFunctor() {
    if constexpr(std::is_constructible_v<StringView, T>) {
        return [](StringView str) { return Any(str); };
    } else if constexpr(std::is_same_v<behaviortree::AnyTypeAllowed, T> || std::is_enum_v<T>) {
        return {};
    } else {
        return [](StringView str) { return Any(ConvertFromString<T>(str)); };
    }
}

template<>
[[nodiscard]] inline StringConverter GetAnyFromStringFunctor<void>() {
    return {};
}

//------------------------------------------------------------------

template<typename T>
constexpr bool IsConvertibleToString() {
    return std::is_convertible_v<T, std::string> ||
           std::is_convertible_v<T, std::string_view>;
}

Expected<std::string> ToJsonString(const Any& refValue);

/**
 * @brief toStr is the reverse operation of convertFromString.
 *
 * If T is a custom type and there is no template specialization,
 * it will try to fall back to toJsonString()
 */
template<typename T>
[[nodiscard]] std::string ToStr(const T& refValue) {
    if constexpr(IsConvertibleToString<T>()) {
        return static_cast<std::string>(refValue);
    } else if constexpr(!std::is_arithmetic_v<T>) {
        if(auto str = ToJsonString(Any(refValue))) {
            return *str;
        }

        throw LogicError(StrCat("Function BT::toStr<T>() not specialized for type [",
                                behaviortree::Demangle(typeid(T)), "]"));
    } else {
        return std::to_string(refValue);
    }
}

template<>
[[nodiscard]] std::string ToStr<bool>(const bool& refValue);

template<>
[[nodiscard]] std::string ToStr<std::string>(const std::string& refValue);

template<>
[[nodiscard]] std::string ToStr<behaviortree::NodeStatus>(const behaviortree::NodeStatus& refNodeStatus);

/**
 * @brief toStr converts NodeStatus to string. Optionally colored.
 */
[[nodiscard]] std::string ToStr(behaviortree::NodeStatus nodeStatus, bool colored);

std::ostream& operator<<(std::ostream& os, const behaviortree::NodeStatus& refNodeStatus);

template<>
[[nodiscard]] std::string ToStr<behaviortree::NodeType>(const behaviortree::NodeType& refType);

std::ostream& operator<<(std::ostream& refOs, const behaviortree::NodeType& refType);

template<>
[[nodiscard]] std::string ToStr<behaviortree::PortDirection>(const behaviortree::PortDirection& refDirection);

std::ostream& operator<<(std::ostream& refOs, const behaviortree::PortDirection& refType);

// Small utility, unless you want to use <boost/algorithm/string.hpp>
[[nodiscard]] std::vector<StringView> SplitString(const StringView& refSrToSplit,
                                                  char delimeter);

template<typename Predicate>
using enable_if = typename std::enable_if<Predicate::value>::type*;

template<typename Predicate>
using enable_if_not = typename std::enable_if<!Predicate::value>::type*;

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

[[nodiscard]] bool IsAllowedPortName(StringView str);

class TypeInfo {
 public:
    template<typename T>
    static TypeInfo Create() {
        return TypeInfo{typeid(T), GetAnyFromStringFunctor<T>()};
    }

    TypeInfo(): m_TypeInfo(typeid(AnyTypeAllowed)), m_TypeStr("AnyTypeAllowed") {}

    TypeInfo(std::type_index type_info, StringConverter conv)
        : m_TypeInfo(type_info), m_Converter(conv), m_TypeStr(behaviortree::Demangle(type_info)) {}

    [[nodiscard]] const std::type_index& Type() const;

    [[nodiscard]] const std::string& TypeName() const;

    [[nodiscard]] Any ParseString(const char* str) const;

    [[nodiscard]] Any ParseString(const std::string& str) const;

    template<typename T>
    [[nodiscard]] Any ParseString(const T&) const {
        // avoid compilation errors
        return {};
    }

    [[nodiscard]] bool IsStronglyTyped() const {
        return m_TypeInfo != typeid(AnyTypeAllowed) && m_TypeInfo != typeid(behaviortree::Any);
    }

    [[nodiscard]] const StringConverter& Converter() const {
        return m_Converter;
    }

 private:
    std::type_index m_TypeInfo;
    StringConverter m_Converter;
    std::string m_TypeStr;
};

class PortInfo: public TypeInfo {
 public:
    PortInfo(PortDirection direction = PortDirection::INOUT)
        : TypeInfo(), m_Direction(direction) {}

    PortInfo(PortDirection direction, std::type_index type_info, StringConverter conv)
        : TypeInfo(type_info, conv), m_Direction(direction) {}

    [[nodiscard]] PortDirection Direction() const;

    void SetDescription(StringView description);

    template<typename T>
    void SetDefaultValue(const T& default_value) {
        m_DefaultValue = Any(default_value);
        try {
            m_DefaultValueStr = behaviortree::ToStr(default_value);
        } catch(LogicError&) {}
    }

    [[nodiscard]] const std::string& Description() const;

    [[nodiscard]] const Any& DefaultValue() const;

    [[nodiscard]] const std::string& DefaultValueString() const;

 private:
    PortDirection m_Direction;
    std::string m_Description;
    Any m_DefaultValue;
    std::string m_DefaultValueStr;
};

template<typename T = AnyTypeAllowed>
[[nodiscard]] std::pair<std::string, PortInfo> CreatePort(PortDirection direction,
                                                          StringView name,
                                                          StringView description = {}) {
    auto sname = static_cast<std::string>(name);
    if(!IsAllowedPortName(sname)) {
        throw RuntimeError(
                "The name of a port must not be `name` or `ID` "
                "and must start with an alphabetic character. "
                "Underscore is reserved.");
    }

    std::pair<std::string, PortInfo> out;

    if(std::is_same<T, void>::value) {
        out = {sname, PortInfo(direction)};
    } else {
        out = {sname, PortInfo(direction, typeid(T), GetAnyFromStringFunctor<T>())};
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
[[nodiscard]] inline std::pair<std::string, PortInfo>
InputPort(StringView name, StringView description = {}) {
    return CreatePort<T>(PortDirection::INPUT, name, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::OUTPUT,...)
 *
 *  @param name the name of the port
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo>
OutputPort(StringView name, StringView description = {}) {
    return CreatePort<T>(PortDirection::OUTPUT, name, description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INOUT,...)
 *
 *  @param name the name of the port
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo>
BidirectionalPort(StringView name, StringView description = {}) {
    return CreatePort<T>(PortDirection::INOUT, name, description);
}
//----------

namespace details {

template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo>
PortWithDefault(PortDirection direction, StringView name, const DefaultT& refDefaultValue,
                StringView description) {
    static_assert(IsConvertibleToString<DefaultT>() || std::is_convertible_v<T, DefaultT> ||
                          std::is_constructible_v<T, DefaultT>,
                  "The default value must be either the same of the port or string");

    auto out = CreatePort<T>(direction, name, description);

    if constexpr(std::is_constructible_v<T, DefaultT>) {
        out.second.SetDefaultValue(T(refDefaultValue));
    } else if constexpr(IsConvertibleToString<DefaultT>()) {
        out.second.SetDefaultValue(std::string(refDefaultValue));
    } else {
        out.second.SetDefaultValue(refDefaultValue);
    }
    return out;
}

}// end namespace details

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INPUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default value of the port, either type T of BlackboardKey
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo>
InputPort(StringView name, const DefaultT& refDefaultValue, StringView description) {
    return details::PortWithDefault<T, DefaultT>(PortDirection::INPUT, name, refDefaultValue,
                                                 description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::INOUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default value of the port, either type T of BlackboardKey
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed, typename DefaultT = T>
[[nodiscard]] inline std::pair<std::string, PortInfo>
BidirectionalPort(StringView name, const DefaultT& refDefaultValue, StringView description) {
    return details::PortWithDefault<T, DefaultT>(PortDirection::INOUT, name, refDefaultValue,
                                                 description);
}

/** Syntactic sugar to invoke CreatePort<T>(PortDirection::OUTPUT,...)
 *  It also sets the PortInfo::defaultValue()
 *
 *  @param name the name of the port
 *  @param default_value default blackboard entry where the output is written
 *  @param description optional human-readable description
 */
template<typename T = AnyTypeAllowed>
[[nodiscard]] inline std::pair<std::string, PortInfo> OutputPort(StringView name,
                                                                 StringView defaultValue,
                                                                 StringView description) {
    if(defaultValue.empty() || defaultValue.front() != '{' || defaultValue.back() != '}') {
        throw LogicError(
                "Output port can only refer to blackboard entries, i.e. use the "
                "syntax '{port_name}'");
    }
    auto out = CreatePort<T>(PortDirection::OUTPUT, name, description);
    out.second.SetDefaultValue(defaultValue);
    return out;
}

//----------

using PortsList = std::unordered_map<std::string, PortInfo>;

template<typename T, typename = void>
struct has_static_method_providedPorts: std::false_type {
};

template<typename T>
struct has_static_method_providedPorts<
        T, typename std::enable_if<
                   std::is_same<decltype(T::providedPorts()), PortsList>::value>::type>
    : std::true_type {
};

template<typename T, typename = void>
struct has_static_method_metadata: std::false_type {
};

template<typename T>
struct has_static_method_metadata<
        T, typename std::enable_if<
                   std::is_same<decltype(T::metadata()), KeyValueVector>::value>::type>
    : std::true_type {
};

template<typename T>
[[nodiscard]] inline PortsList
GetProvidedPorts(enable_if<has_static_method_providedPorts<T>> = nullptr) {
    return T::providedPorts();
}

template<typename T>
[[nodiscard]] inline PortsList
GetProvidedPorts(enable_if_not<has_static_method_providedPorts<T>> = nullptr) {
    return {};
}

using TimePoint = std::chrono::high_resolution_clock::time_point;
using Duration = std::chrono::high_resolution_clock::duration;

}// namespace behaviortree

#endif//BEHAVIORTREE_BASIC_TYPES_H
