module behaviortree.basic_types;

import <charconv>;
import <clocale>;
import <cstdlib>;
import <cstring>;

import behaviortree.json_export;

namespace behaviortree {
template<>
std::string ToStr<NodeStatus>(const NodeStatus &rNodeStatus) {
    switch(rNodeStatus) {
        case NodeStatus::Success: {
            return "SUCCESS";
        } break;
        case NodeStatus::Failure: {
            return "FAILURE";
        } break;
        case NodeStatus::Running: {
            return "RUNNING";
        } break;
        case NodeStatus::Idle: {
            return "IDLE";
        } break;
        case NodeStatus::Skipped: {
            return "SKIPPED";
        } break;
        default: {

        } break;
    }
    return "";
}

template<>
[[nodiscard]] std::string ToStr<bool>(const bool &rValue) {
    return rValue ? "true" : "false";
}

template<>
std::string ToStr<std::string>(const std::string &rValue) {
    return rValue;
}

std::string ToStr(NodeStatus nodeStatus, bool colored) {
    if(!colored) {
        return ToStr(nodeStatus);
    } else {
        switch(nodeStatus) {
            case NodeStatus::Success: {
                return "\x1b[32m"
                       "SUCCESS"
                       "\x1b[0m";// RED
            } break;
            case NodeStatus::Failure: {
                return "\x1b[31m"
                       "FAILURE"
                       "\x1b[0m";// GREEN
            } break;
            case NodeStatus::Running: {
                return "\x1b[33m"
                       "RUNNING"
                       "\x1b[0m";// YELLOW
            } break;
            case NodeStatus::Skipped: {
                return "\x1b[34m"
                       "SKIPPED"
                       "\x1b[0m";// BLUE
            } break;
            case NodeStatus::Idle: {
                return "\x1b[36m"
                       "IDLE"
                       "\x1b[0m";// CYAN
            } break;
            default: {

            } break;
        }
    }
    return "Undefined";
}

template<>
std::string ToStr<PortDirection>(const PortDirection &rDirection) {
    switch(rDirection) {
        case PortDirection::Input: {
            return "Input";
        } break;
        case PortDirection::Output: {
            return "Output";
        } break;
        case PortDirection::InOut: {
            return "InOut";
        } break;
        default: {
            return "InOut";
        } break;
    }
}

template<>
std::string ToStr<NodeType>(const NodeType &rNodeType) {
    switch(rNodeType) {
        case NodeType::Action: {
            return "Action";
        } break;
        case NodeType::Condition: {
            return "Condition";
        } break;
        case NodeType::Decorator: {
            return "Decorator";
        } break;
        case NodeType::Control: {
            return "Control";
        } break;
        case NodeType::Subtree: {
            return "Subtree";
        } break;
        default: {
            return "Undefined";
        } break;
    }
}

template<>
std::string ConvertFromString<std::string>(std::string_view str) {
    return std::string(str.data(), str.size());
}

template<>
int64_t ConvertFromString<int64_t>(std::string_view str) {
    int64_t result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if(ec != std::errc()) {
        throw util::RuntimeError(util::StrCat("Can't Convert string [", str, "] to integer"));
    }
    return result;
}

template<>
uint64_t ConvertFromString<uint64_t>(std::string_view str) {
    uint64_t result = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
    if(ec != std::errc()) {
        throw util::RuntimeError(util::StrCat("Can't Convert string [", str, "] to integer"));
    }
    return result;
}

template<typename T>
T ConvertWithBoundCheck(std::string_view str) {
    auto res = ConvertFromString<int64_t>(str);
    if(res < std::numeric_limits<T>::lowest() || res > std::numeric_limits<T>::max()) {
        throw util::RuntimeError(util::StrCat("Value out of bound when converting [", str, "] to integer"));
    }
    return res;
}

template<>
int8_t ConvertFromString<int8_t>(std::string_view str) {
    return ConvertWithBoundCheck<int8_t>(str);
}

template<>
int16_t ConvertFromString<int16_t>(std::string_view str) {
    return ConvertWithBoundCheck<int16_t>(str);
}

template<>
int32_t ConvertFromString<int32_t>(std::string_view str) {
    return ConvertWithBoundCheck<int32_t>(str);
}

template<>
uint8_t ConvertFromString<uint8_t>(std::string_view str) {
    return ConvertWithBoundCheck<uint8_t>(str);
}

template<>
uint16_t ConvertFromString<uint16_t>(std::string_view str) {
    return ConvertWithBoundCheck<uint16_t>(str);
}

template<>
uint32_t ConvertFromString<uint32_t>(std::string_view str) {
    return ConvertWithBoundCheck<uint32_t>(str);
}

template<>
double ConvertFromString<double>(std::string_view str) {
    // see issue #120
    // http://quick-bench.com/DWaXRWnxtxvwIMvZy2DxVPEKJnE

    std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");
    double val = std::stod(str.data());
    setlocale(LC_NUMERIC, oldLocale.c_str());
    return val;
}

template<>
float ConvertFromString<float>(std::string_view str) {
    std::string oldLocale = setlocale(LC_NUMERIC, nullptr);
    setlocale(LC_NUMERIC, "C");
    float val = std::stof(str.data());
    setlocale(LC_NUMERIC, oldLocale.c_str());
    return val;
}

template<>
std::vector<int32_t> ConvertFromString<std::vector<int32_t>>(std::string_view str) {
    auto partVec = SplitString(str, ';');
    std::vector<int> output;
    output.reserve(partVec.size());
    for(const std::string_view &rPart: partVec) {
        output.push_back(ConvertFromString<int32_t>(rPart));
    }
    return output;
}

template<>
std::vector<double> ConvertFromString<std::vector<double>>(std::string_view str) {
    auto partVec = SplitString(str, ';');
    std::vector<double> output;
    output.reserve(partVec.size());
    for(const std::string_view &rPart: partVec) {
        output.push_back(ConvertFromString<double>(rPart));
    }
    return output;
}

template<>
std::vector<std::string> ConvertFromString<std::vector<std::string>>(std::string_view str) {
    auto partVec = SplitString(str, ';');
    std::vector<std::string> output;
    output.reserve(partVec.size());
    for(const std::string_view &rPart: partVec) {
        output.push_back(ConvertFromString<std::string>(rPart));
    }
    return output;
}

template<>
bool ConvertFromString<bool>(std::string_view str) {
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
    throw util::RuntimeError("convertFromString(): invalid bool conversion");
}

template<>
NodeStatus ConvertFromString<NodeStatus>(std::string_view str) {
    if(str == "IDLE") {
        return NodeStatus::Idle;
    }
    if(str == "RUNNING") {
        return NodeStatus::Running;
    }
    if(str == "SUCCESS") {
        return NodeStatus::Success;
    }
    if(str == "FAILURE") {
        return NodeStatus::Failure;
    }
    if(str == "SKIPPED") {
        return NodeStatus::Skipped;
    }

    throw util::RuntimeError(std::string("Cannot Convert this to NodeStatus: ") + static_cast<std::string>(str));
}

template<>
NodeType ConvertFromString<NodeType>(std::string_view str) {
    if(str == "Action") {
        return NodeType::Action;
    }
    if(str == "Condition") {
        return NodeType::Condition;
    }
    if(str == "Control") {
        return NodeType::Control;
    }
    if(str == "Decorator") {
        return NodeType::Decorator;
    }
    if(str == "Subtree") {
        return NodeType::Subtree;
    }
    return NodeType::Undefined;
}

template<>
PortDirection ConvertFromString<PortDirection>(std::string_view str) {
    if(str == "Input" || str == "INPUT") {
        return PortDirection::Input;
    }
    if(str == "Output" || str == "OUTPUT") {
        return PortDirection::Output;
    }
    if(str == "InOut" || str == "INOUT") {
        return PortDirection::InOut;
    }
    throw util::RuntimeError(std::string("Cannot Convert this to PortDirection: ") + static_cast<std::string>(str));
}

std::ostream &operator<<(std::ostream &rOS, const NodeType &rNodeType) {
    rOS << ToStr(rNodeType);
    return rOS;
}

std::ostream &operator<<(std::ostream &rOS, const NodeStatus &rNodeStatus) {
    rOS << ToStr(rNodeStatus);
    return rOS;
}

std::ostream &operator<<(std::ostream &rOS, const PortDirection &rPortDirection) {
    rOS << ToStr(rPortDirection);
    return rOS;
}

std::vector<std::string_view> SplitString(const std::string_view &rStrToSplit, char delimeter) {
    std::vector<std::string_view> splittedStringVec;
    splittedStringVec.reserve(4);

    size_t pos{0};
    while(pos < rStrToSplit.size()) {
        size_t newPos = rStrToSplit.find_first_of(delimeter, pos);
        if(newPos == std::string::npos) {
            newPos = rStrToSplit.size();
        }
        const auto sv = std::string_view{&rStrToSplit.data()[pos], newPos - pos};
        splittedStringVec.push_back(sv);
        pos = newPos + 1;
    }
    return splittedStringVec;
}

PortDirection PortInfo::Direction() const {
    return m_direction;
}

const std::type_index &TypeInfo::Type() const {
    return m_typeInfo;
}

const std::string &TypeInfo::TypeName() const {
    return m_typeStr;
}

Any TypeInfo::ParseString(const char *pStr) const {
    if(m_converter) {
        return m_converter(pStr);
    }
    return {};
}

Any TypeInfo::ParseString(const std::string &rStr) const {
    if(m_converter) {
        return m_converter(rStr);
    }
    return {};
}

void PortInfo::SetDescription(std::string_view description) {
    m_description = static_cast<std::string>(description);
}

const std::string &PortInfo::Description() const {
    return m_description;
}

const Any &PortInfo::DefaultValue() const {
    return m_defaultValue;
}

const std::string &PortInfo::DefaultValueString() const {
    return m_defaultValueStr;
}

bool IsAllowedPortName(std::string_view str) {
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

Any ConvertFromJson(std::string_view jsonText, std::type_index type) {
    nlohmann::json json = nlohmann::json::parse(jsonText);
    auto res = JsonExporter::Get().FromJson(json, type);
    if(!res) {
        throw std::runtime_error(res.error());
    }
    return res->first;
}

Expected<std::string> ToJsonString(const Any &rValue) {
    nlohmann::json json;
    if(JsonExporter::Get().ToJson(rValue, json)) {
        return util::StrCat("json:", json.dump());
    }
    return nonstd::make_unexpected("ToJsonString failed");
}

bool StartWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && strncmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool StartWith(std::string_view str, char prefix) {
    return str.size() >= 1 && str[0] == prefix;
}

}// namespace behaviortree

// module behaviortree.basic_types;
