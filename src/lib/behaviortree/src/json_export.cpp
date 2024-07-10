#include "behaviortree/json_export.h"

namespace behaviortree {

JsonExporter &JsonExporter::Get() {
    static JsonExporter globalInstance;
    return globalInstance;
}

bool JsonExporter::ToJson(const Any &refAny, nlohmann::json &refDst) const {
    nlohmann::json json;
    auto const &refType = refAny.CastedType();

    if(refAny.IsString()) {
        refDst = refAny.Cast<std::string>();
    } else if(refType == typeid(int64_t)) {
        refDst = refAny.Cast<int64_t>();
    } else if(refType == typeid(uint64_t)) {
        refDst = refAny.Cast<uint64_t>();
    } else if(refType == typeid(double)) {
        refDst = refAny.Cast<double>();
    } else {
        auto it = m_ToJsonConverters.find(refType);
        if(it != m_ToJsonConverters.end()) {
            it->second(refAny, refDst);
        } else {
            return false;
        }
    }
    return true;
}

JsonExporter::ExpectedEntry JsonExporter::FromJson(
        const nlohmann::json &refSource
) const {
    if(refSource.is_null()) {
        return nonstd::make_unexpected("json object is null");
    }
    if(refSource.is_string()) {
        return Entry{
                behaviortree::Any(refSource.get<std::string>()),
                behaviortree::TypeInfo::Create<std::string>()
        };
    }
    if(refSource.is_number_unsigned()) {
        return Entry{
                behaviortree::Any(refSource.get<uint64_t>()),
                behaviortree::TypeInfo::Create<uint64_t>()
        };
    }
    if(refSource.is_number_integer()) {
        return Entry{
                behaviortree::Any(refSource.get<int64_t>()),
                behaviortree::TypeInfo::Create<int64_t>()
        };
    }
    if(refSource.is_number_float()) {
        return Entry{
                behaviortree::Any(refSource.get<double>()),
                behaviortree::TypeInfo::Create<double>()
        };
    }
    if(refSource.is_boolean()) {
        return Entry{
                behaviortree::Any(refSource.get<bool>()),
                behaviortree::TypeInfo::Create<bool>()
        };
    }

    if(!refSource.contains("__type")) {
        return nonstd::make_unexpected("Missing field '__type'");
    }
    auto typeIt = m_TypeNames.find(refSource["__type"]);
    if(typeIt == m_TypeNames.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    auto funcIt = m_FromJsonConverters.find(typeIt->second.Type());
    if(funcIt == m_FromJsonConverters.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    return funcIt->second(refSource);
}

JsonExporter::ExpectedEntry JsonExporter::FromJson(
        const nlohmann::json &refSource, std::type_index type
) const {
    auto funcIt = m_FromJsonConverters.find(type);
    if(funcIt == m_FromJsonConverters.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    return funcIt->second(refSource);
}

}// namespace behaviortree
