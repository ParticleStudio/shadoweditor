#include "behaviortree/basic_types.h"

#include <charconv>
#include <clocale>
#include <cstdlib>
#include <cstring>

#include "behaviortree/json_export.h"

namespace behaviortree {
template<>
std::string ToStr<NodeStatus>(const NodeStatus& refNodeStatus) {
    switch(refNodeStatus) {
        case NodeStatus::SUCCESS:
            return "SUCCESS";
        case NodeStatus::FAILURE:
            return "FAILURE";
        case NodeStatus::RUNNING:
            return "RUNNING";
        case NodeStatus::IDLE:
            return "IDLE";
        case NodeStatus::SKIPPED:
            return "SKIPPED";
    }
    return "";
}

template<>
[[nodiscard]] std::string ToStr<bool>(const bool& refValue) {
    return refValue ? "true" : "false";
}

template<>
std::string ToStr<std::string>(const std::string& refValue) {
    return refValue;
}

std::string ToStr(NodeStatus nodeStatus, bool colored) {
    if(!colored) {
        return ToStr(nodeStatus);
    } else {
        switch(nodeStatus) {
            case NodeStatus::SUCCESS:
                return "\x1b[32m"
                       "SUCCESS"
                       "\x1b[0m";// RED
            case NodeStatus::FAILURE:
                return "\x1b[31m"
                       "FAILURE"
                       "\x1b[0m";// GREEN
            case NodeStatus::RUNNING:
                return "\x1b[33m"
                       "RUNNING"
                       "\x1b[0m";// YELLOW
            case NodeStatus::SKIPPED:
                return "\x1b[34m"
                       "SKIPPED"
                       "\x1b[0m";// BLUE
            case NodeStatus::IDLE:
                return "\x1b[36m"
                       "IDLE"
                       "\x1b[0m";// CYAN
        }
    }
    return "Undefined";
}

template<>
std::string ToStr<PortDirection>(const PortDirection& refDirection) {
    switch(refDirection) {
        case PortDirection::INPUT:
            return "Input";
        case PortDirection::OUTPUT:
            return "Output";
        case PortDirection::INOUT:
            return "InOut";
    }
    return "InOut";
}

template<>
std::string ToStr<NodeType>(const NodeType& refNodeType) {
    switch(refNodeType) {
        case NodeType::ACTION:
            return "Action";
        case NodeType::CONDITION:
            return "Condition";
        case NodeType::DECORATOR:
            return "Decorator";
        case NodeType::CONTROL:
            return "Control";
        case NodeType::SUBTREE:
            return "SubTree";
        default:
            return "Undefined";
    }
}

template<>
std::string ConvertFromString<std::string>(StringView str) {
    return std::string(str.data(), str.size());
}

template<>
int64_t ConvertFromString<int64_t>(StringView str) {
    long result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if(ec != std::errc()) {
        throw RuntimeError(StrCat("Can't Convert string [", str, "] to integer"));
    }
    return result;
}

template<>
uint64_t ConvertFromString<uint64_t>(StringView str) {
    unsigned long result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if(ec != std::errc()) {
        throw RuntimeError(StrCat("Can't Convert string [", str, "] to integer"));
    }
    return result;
}

template<typename T>
T ConvertWithBoundCheck(StringView str) {
    auto res = ConvertFromString<int64_t>(str);
    if(res < std::numeric_limits<T>::lowest() || res > std::numeric_limits<T>::max()) {
        throw RuntimeError(
                StrCat("Value out of bound when converting [", str, "] to integer"));
    }
    return res;
}

template<>
int8_t ConvertFromString<int8_t>(StringView str) {
    return ConvertWithBoundCheck<int8_t>(str);
}

template<>
int16_t ConvertFromString<int16_t>(StringView str) {
    return ConvertWithBoundCheck<int16_t>(str);
}

template<>
int32_t ConvertFromString<int32_t>(StringView str) {
    return ConvertWithBoundCheck<int32_t>(str);
}

template<>
uint8_t ConvertFromString<uint8_t>(StringView str) {
    return ConvertWithBoundCheck<uint8_t>(str);
}

template<>
uint16_t ConvertFromString<uint16_t>(StringView str) {
    return ConvertWithBoundCheck<uint16_t>(str);
}

template<>
uint32_t ConvertFromString<uint32_t>(StringView str) {
    return ConvertWithBoundCheck<uint32_t>(str);
}

template<>
double ConvertFromString<double>(StringView str) {
    // see issue #120
    // http://quick-bench.com/DWaXRWnxtxvwIMvZy2DxVPEKJnE

    std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");
    double val = std::stod(str.data());
    setlocale(LC_NUMERIC, oldLocale.c_str());
    return val;
}

template<>
float ConvertFromString<float>(StringView str) {
    std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");
    float val = std::stof(str.data());
    setlocale(LC_NUMERIC, oldLocale.c_str());
    return val;
}

template<>
std::vector<int> ConvertFromString<std::vector<int>>(StringView str) {
    auto parts = SplitString(str, ';');
    std::vector<int> output;
    output.reserve(parts.size());
    for(const StringView& refPart: parts) {
        output.push_back(ConvertFromString<int>(refPart));
    }
    return output;
}

template<>
std::vector<double> ConvertFromString<std::vector<double>>(StringView str) {
    auto parts = SplitString(str, ';');
    std::vector<double> output;
    output.reserve(parts.size());
    for(const StringView& refPart: parts) {
        output.push_back(ConvertFromString<double>(refPart));
    }
    return output;
}

template<>
std::vector<std::string> ConvertFromString<std::vector<std::string>>(StringView str) {
    auto parts = SplitString(str, ';');
    std::vector<std::string> output;
    output.reserve(parts.size());
    for(const StringView& refPart: parts) {
        output.push_back(ConvertFromString<std::string>(refPart));
    }
    return output;
}

template<>
bool ConvertFromString<bool>(StringView str) {
    if(str.size() == 1) {
        if(str[0] == '0') {
            return false;
        }
        if(str[0] == '1') {
            return true;
        }
    } else if(str.size() == 4) {
        if(str == "true" || str == "TRUE" || str == "True") {
            return true;
        }
    } else if(str.size() == 5) {
        if(str == "false" || str == "FALSE" || str == "False") {
            return false;
        }
    }
    throw RuntimeError("convertFromString(): invalid bool conversion");
}

template<>
NodeStatus ConvertFromString<NodeStatus>(StringView str) {
    if(str == "IDLE")
        return NodeStatus::IDLE;
    if(str == "RUNNING")
        return NodeStatus::RUNNING;
    if(str == "SUCCESS")
        return NodeStatus::SUCCESS;
    if(str == "FAILURE")
        return NodeStatus::FAILURE;
    if(str == "SKIPPED")
        return NodeStatus::SKIPPED;

    throw RuntimeError(std::string("Cannot Convert this to NodeStatus: ") +
                       static_cast<std::string>(str));
}

template<>
NodeType ConvertFromString<NodeType>(StringView str) {
    if(str == "Action")
        return NodeType::ACTION;
    if(str == "Condition")
        return NodeType::CONDITION;
    if(str == "Control")
        return NodeType::CONTROL;
    if(str == "Decorator")
        return NodeType::DECORATOR;
    if(str == "SubTree")
        return NodeType::SUBTREE;
    return NodeType::UNDEFINED;
}

template<>
PortDirection ConvertFromString<PortDirection>(StringView str) {
    if(str == "Input" || str == "INPUT")
        return PortDirection::INPUT;
    if(str == "Output" || str == "OUTPUT")
        return PortDirection::OUTPUT;
    if(str == "InOut" || str == "INOUT")
        return PortDirection::INOUT;
    throw RuntimeError(std::string("Cannot Convert this to PortDirection: ") +
                       static_cast<std::string>(str));
}

std::ostream& operator<<(std::ostream& refOS, const NodeType& refNodeType) {
    refOS << ToStr(refNodeType);
    return refOS;
}

std::ostream& operator<<(std::ostream& refOS, const NodeStatus& refNodeStatus) {
    refOS << ToStr(refNodeStatus);
    return refOS;
}

std::ostream& operator<<(std::ostream& refOS, const PortDirection& refPortDirection) {
    refOS << ToStr(refPortDirection);
    return refOS;
}

std::vector<StringView> SplitString(const StringView& refStrToSplit, char delimeter) {
    std::vector<StringView> splittedStrings;
    splittedStrings.reserve(4);

    size_t pos{0};
    while(pos < refStrToSplit.size()) {
        size_t newPos = refStrToSplit.find_first_of(delimeter, pos);
        if(newPos == std::string::npos) {
            newPos = refStrToSplit.size();
        }
        const auto sv = StringView{&refStrToSplit.data()[pos], newPos - pos};
        splittedStrings.push_back(sv);
        pos = newPos + 1;
    }
    return splittedStrings;
}

PortDirection PortInfo::Direction() const {
    return m_Direction;
}

const std::type_index& TypeInfo::Type() const {
    return m_TypeInfo;
}

const std::string& TypeInfo::TypeName() const {
    return m_TypeStr;
}

Any TypeInfo::ParseString(const char* refStr) const {
    if(m_Converter) {
        return m_Converter(refStr);
    }
    return {};
}

Any TypeInfo::ParseString(const std::string& refStr) const {
    if(m_Converter) {
        return m_Converter(refStr);
    }
    return {};
}

void PortInfo::SetDescription(StringView description) {
    m_Description = static_cast<std::string>(description);
}

const std::string& PortInfo::Description() const {
    return m_Description;
}

const Any& PortInfo::DefaultValue() const {
    return m_DefaultValue;
}

const std::string& PortInfo::DefaultValueString() const {
    return m_DefaultValueStr;
}

bool IsAllowedPortName(StringView str) {
    if(str == "_autoremap") {
        return true;
    }
    if(str.empty()) {
        return false;
    }
    const char firstChar = str.data()[0];
    if(!std::isalpha(firstChar)) {
        return false;
    }
    if(str == "name" || str == "ID") {
        return false;
    }
    return true;
}

Any ConvertFromJSON(StringView jsonText, std::type_index type) {
    nlohmann::json json = nlohmann::json::parse(jsonText);
    auto res = JsonExporter::Get().FromJson(json, type);
    if(!res) {
        throw std::runtime_error(res.error());
    }
    return res->first;
}

Expected<std::string> ToJsonString(const Any& refValue) {
    nlohmann::json json;
    if(JsonExporter::Get().ToJson(refValue, json)) {
        return StrCat("json:", json.dump());
    }
    return nonstd::make_unexpected("toJsonString failed");
}

bool StartWith(StringView str, StringView prefix) {
    return str.size() >= prefix.size() &&
           strncmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool StartWith(StringView str, char prefix) {
    return str.size() >= 1 && str[0] == prefix;
}

}// namespace behaviortree
