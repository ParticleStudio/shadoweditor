module behaviortree.json_export;

namespace behaviortree {

JsonExporter &JsonExporter::Get() {
    static JsonExporter globalInstance;
    return globalInstance;
}

bool JsonExporter::ToJson(const Any &rAny, nlohmann::json &rDst) const {
    nlohmann::json json;
    auto const &rType = rAny.CastedType();

    if(rAny.IsString()) {
        rDst = rAny.Cast<std::string>();
    } else if(rType == typeid(int64_t)) {
        rDst = rAny.Cast<int64_t>();
    } else if(rType == typeid(uint64_t)) {
        rDst = rAny.Cast<uint64_t>();
    } else if(rType == typeid(double)) {
        rDst = rAny.Cast<double>();
    } else {
        auto iter = m_toJsonConverterMap.find(rType);
        if(iter != m_toJsonConverterMap.end()) {
            iter->second(rAny, rDst);
        } else {
            return false;
        }
    }
    return true;
}

JsonExporter::ExpectedEntry JsonExporter::FromJson(const nlohmann::json &rSource) const {
    if(rSource.is_null()) {
        return nonstd::make_unexpected("json object is null");
    }
    if(rSource.is_string()) {
        return Entry{
                behaviortree::Any(rSource.get<std::string>()),
                behaviortree::TypeInfo::Create<std::string>()
        };
    }
    if(rSource.is_number_unsigned()) {
        return Entry{
                behaviortree::Any(rSource.get<uint64_t>()),
                behaviortree::TypeInfo::Create<uint64_t>()
        };
    }
    if(rSource.is_number_integer()) {
        return Entry{
                behaviortree::Any(rSource.get<int64_t>()),
                behaviortree::TypeInfo::Create<int64_t>()
        };
    }
    if(rSource.is_number_float()) {
        return Entry{
                behaviortree::Any(rSource.get<double>()),
                behaviortree::TypeInfo::Create<double>()
        };
    }
    if(rSource.is_boolean()) {
        return Entry{
                behaviortree::Any(rSource.get<bool>()),
                behaviortree::TypeInfo::Create<bool>()
        };
    }

    if(!rSource.contains("__type")) {
        return nonstd::make_unexpected("Missing field '__type'");
    }
    auto typeNameIter = m_typeNameMap.find(rSource["__type"]);
    if(typeNameIter == m_typeNameMap.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    auto funcIter = m_fromJsonConverterMap.find(typeNameIter->second.Type());
    if(funcIter == m_fromJsonConverterMap.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    return funcIter->second(rSource);
}

JsonExporter::ExpectedEntry JsonExporter::FromJson(const nlohmann::json &rSource, std::type_index type) const {
    auto funcIter = m_fromJsonConverterMap.find(type);
    if(funcIter == m_fromJsonConverterMap.end()) {
        return nonstd::make_unexpected("Type not found in registered list");
    }
    return funcIter->second(rSource);
}

}// namespace behaviortree

// module behaviortree.json_export;
